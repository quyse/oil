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

BEGIN_INANITY_OIL

static ptr<File> GetBlockName(size_t index)
{
	ptr<MemoryStream> stream = NEW(MemoryStream(9));
	StreamWriter writer(stream);
	writer.WriteShortly(index);
	return stream->ToFile();
}

//*** FileEntitySchemeInputStream class

FileEntitySchemeInputStream::FileEntitySchemeInputStream(ptr<Entity> entity)
: entity(entity), currentBlockIndex(0), currentBlockOffset(0)
{
	BEGIN_TRY();

	// read descriptor
	ptr<File> descriptorFile = entity->ReadData(nullptr);
	if(!descriptorFile)
		THROW("No descriptor");

	descriptorReader = NEW(StreamReader(NEW(FileInputStream(descriptorFile))));

	totalSize = descriptorReader->ReadShortly();
	remainingSize = totalSize;

	hashStream = NEW(Crypto::WhirlpoolStream());

	END_TRY("Can't create input stream for file entity");
}

bigsize_t FileEntitySchemeInputStream::GetSize() const
{
	return totalSize;
}

size_t FileEntitySchemeInputStream::Read(void* data, size_t size)
{
	BEGIN_TRY();

	char* dataPtr = (char*)data;

	// don't allow to read more than remaining size
	if((bigsize_t)size > remainingSize)
		size = (size_t)remainingSize;

	while(size)
	{
		size_t currentBlockSize = currentBlockFile ? currentBlockFile->GetSize() : 0;

		if(currentBlockOffset >= currentBlockSize)
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

		size_t sizeToCopy = currentBlockSize - currentBlockOffset;
		if(sizeToCopy > size)
			sizeToCopy = size;
		memcpy(dataPtr, currentBlockFile->GetData(), sizeToCopy);
		dataPtr += sizeToCopy;
		size -= sizeToCopy;
		remainingSize -= sizeToCopy;
	}

	return dataPtr - (char*)data;

	END_TRY("Can't read data from file entity");
}

//*** FileEntitySchemeOutputStream class

FileEntitySchemeOutputStream::FileEntitySchemeOutputStream(ptr<Action> action, ptr<Entity> entity, size_t blockSize)
: action(action), entity(entity), blockSize(blockSize), currentBlockIndex(0), currentBlockSize(0)
{
	descriptorStream = NEW(MemoryStream());
	descriptorWriter = NEW(StreamWriter(descriptorStream));

	hashStream = NEW(Crypto::WhirlpoolStream());
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
	descriptorWriter->Write(hashFile);

	// write block to entity
	entity->WriteData(action, GetBlockName(currentBlockIndex), blockFile);

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
		{
			currentBlockStream = NEW(MemoryStream(blockSize));
			currentBlockSize = 0;
		}

		// write data
		size_t sizeToWrite = blockSize - currentBlockSize;
		if(sizeToWrite > size)
			sizeToWrite = size;
		currentBlockStream->Write(dataPtr, sizeToWrite);
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

	// write descriptor
	entity->WriteData(action, nullptr, descriptorStream->ToFile());
}

END_INANITY_OIL
