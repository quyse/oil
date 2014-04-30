#include "FileEntityScheme.hpp"
#include "Entity.hpp"
#include "Action.hpp"
#include "../inanity/crypto/WhirlpoolStream.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/MemoryStream.hpp"
#include "../inanity/MemoryFile.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/Exception.hpp"
#include <cstring>

BEGIN_INANITY_OIL

static ptr<File> GetBlockName(size_t index)
{
	ptr<MemoryStream> stream = NEW(MemoryStream(9));
	StreamWriter writer(stream);
	writer.WriteShortly(index);
	return stream->ToFile();
}

/* Descriptor format, version 1.
descriptor version = 1 (shortly)
total size (shortly big)
total hash (64-byte WHIRLPOOL hash)
block hash[blocks count] (64-byte WHIRLPOOL hash)
*/

//*** FileEntitySchemeInputStream class

FileEntitySchemeInputStream::FileEntitySchemeInputStream(ptr<Entity> entity)
: entity(entity), currentBlockIndex(0), currentBlockOffset(0), ended(false)
{
	BEGIN_TRY();

	// read descriptor
	ptr<File> descriptorFile = entity->ReadData(nullptr);
	if(!descriptorFile)
		THROW("No descriptor");

	descriptorReader = NEW(StreamReader(NEW(FileInputStream(descriptorFile))));

	// read & check the version
	size_t version = descriptorReader->ReadShortly();
	// for now only version 1 is supported
	if(version != 1)
		THROW("Invalid descriptor version");
	totalSize = descriptorReader->ReadShortlyBig();
	remainingSize = totalSize;

	hashStream = NEW(Crypto::WhirlpoolStream());
	totalHashStream = NEW(Crypto::WhirlpoolStream());

	// read total hash
	totalHashFromDescriptor = descriptorReader->Read(totalHashStream->GetHashSize());

	END_TRY("Can't create input stream for file entity");
}

bigsize_t FileEntitySchemeInputStream::GetSize() const
{
	return totalSize;
}

size_t FileEntitySchemeInputStream::Read(void* data, size_t size)
{
	BEGIN_TRY();

	if(ended)
		return 0;

	char* dataPtr = (char*)data;

	// don't allow to read more than remaining size
	if((bigsize_t)size > remainingSize)
		size = (size_t)remainingSize;

	while(size)
	{
		if(!currentBlockFile || currentBlockOffset >= currentBlockFile->GetSize())
		{
			// read next block

			// reset offset
			currentBlockOffset = 0;

			// get the block
			currentBlockFile = entity->ReadData(GetBlockName(currentBlockIndex));
			if(!currentBlockFile)
				THROW("Can't read next block");

			// increment index for next block
			++currentBlockIndex;

			// write it into total hash stream
			totalHashStream->Write(currentBlockFile);

			// read and check the hash
			size_t hashSize = hashStream->GetHashSize();
			ptr<File> hashFromDescriptor = descriptorReader->Read(hashSize);

			hashStream->Reset();
			hashStream->Write(currentBlockFile);
			hashStream->End();
			ptr<File> hashFromBlock = NEW(MemoryFile(hashSize));
			hashStream->GetHash(hashFromBlock->GetData());

			if(memcmp(hashFromDescriptor->GetData(), hashFromBlock->GetData(), hashSize) != 0)
				THROW("Wrong hash");
		}

		size_t sizeToCopy = currentBlockFile->GetSize() - currentBlockOffset;
		if(sizeToCopy > size)
			sizeToCopy = size;
		memcpy(dataPtr, (const char*)currentBlockFile->GetData() + currentBlockOffset, sizeToCopy);
		dataPtr += sizeToCopy;
		size -= sizeToCopy;
		currentBlockOffset += sizeToCopy;
		remainingSize -= sizeToCopy;
	}

	// if there is no more data, check that data is actually ends
	if(remainingSize <= 0)
	{
		ended = true;

		// check total hash
		totalHashStream->End();
		size_t hashSize = totalHashStream->GetHashSize();
		ptr<File> totalHash = NEW(MemoryFile(hashSize));
		totalHashStream->GetHash(totalHash->GetData());
		if(memcmp(totalHash->GetData(), totalHashFromDescriptor->GetData(), hashSize) != 0)
			THROW("Wrong total hash");

		// check that last block is over
		if(currentBlockFile && currentBlockOffset < currentBlockFile->GetSize())
			THROW("Last block is not over");

		// check that last block is last
		descriptorReader->ReadEnd();
	}

	return dataPtr - (char*)data;

	END_TRY("Can't read data from file entity");
}

//*** FileEntitySchemeOutputStream class

FileEntitySchemeOutputStream::FileEntitySchemeOutputStream(ptr<Action> action, ptr<Entity> entity, size_t blockSize)
: action(action), entity(entity), blockSize(blockSize), totalSize(0), currentBlockIndex(0), currentBlockSize(0)
{
	descriptorHashStream = NEW(MemoryStream());
	descriptorHashWriter = NEW(StreamWriter(descriptorHashStream));

	hashStream = NEW(Crypto::WhirlpoolStream());
	totalHashStream = NEW(Crypto::WhirlpoolStream());
}

void FileEntitySchemeOutputStream::WriteData(ptr<File> name, ptr<File> value)
{
	if(action)
		entity->WriteData(action, name, value);
	else
		entity->WriteDataStraight(name, value);
}

void FileEntitySchemeOutputStream::FinishCurrentBlock()
{
	// get block
	ptr<File> blockFile = currentBlockStream->ToFile();

	// calculate block hash
	hashStream->Reset();
	hashStream->Write(blockFile);
	hashStream->End();
	ptr<File> hashFile = NEW(MemoryFile(hashStream->GetHashSize()));
	hashStream->GetHash(hashFile->GetData());

	// write hash to descriptor
	descriptorHashWriter->Write(hashFile);

	// write block to entity
	WriteData(GetBlockName(currentBlockIndex), blockFile);

	// increase total size
	totalSize += blockFile->GetSize();

	// reset current block
	currentBlockStream = nullptr;
	currentBlockSize = 0;
	++currentBlockIndex;
}

void FileEntitySchemeOutputStream::Write(const void* data, size_t size)
{
	BEGIN_TRY();

	const char* dataPtr = (const char*)data;

	while(size)
	{
		// if there's no current block stream, create it
		if(!currentBlockStream)
			currentBlockStream = NEW(MemoryStream(blockSize));

		// write data
		size_t sizeToWrite = blockSize - currentBlockSize;
		if(sizeToWrite > size)
			sizeToWrite = size;
		currentBlockStream->Write(dataPtr, sizeToWrite);
		// write it also to total hash stream
		totalHashStream->Write(dataPtr, sizeToWrite);
		// move pointers and counters
		dataPtr += sizeToWrite;
		size -= sizeToWrite;
		currentBlockSize += sizeToWrite;


		// if block is over, finish it
		if(currentBlockSize >= blockSize)
			FinishCurrentBlock();
	}

	END_TRY("Can't write data to file entity");
}

void FileEntitySchemeOutputStream::End()
{
	// if there's current block, finish it
	if(currentBlockStream)
		FinishCurrentBlock();

	// get hashes file
	ptr<File> hashes = descriptorHashStream->ToFile();
	descriptorHashStream = nullptr;

	// get total hash file
	ptr<File> totalHash = NEW(MemoryFile(totalHashStream->GetHashSize()));
	totalHashStream->End();
	totalHashStream->GetHash(totalHash->GetData());

	// write descriptor, version 1
	ptr<MemoryStream> descriptorStream = NEW(MemoryStream(hashes->GetSize() + 9));
	StreamWriter writer(descriptorStream);
	writer.WriteShortly(1); // descriptor version
	writer.WriteShortlyBig(totalSize);
	writer.Write(totalHash);
	writer.Write(hashes);

	// store descriptor
	WriteData(nullptr, descriptorStream->ToFile());
}

END_INANITY_OIL
