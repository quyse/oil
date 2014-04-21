#ifndef ___INANITY_OIL_FILE_ENTITY_SCHEME_HPP___
#define ___INANITY_OIL_FILE_ENTITY_SCHEME_HPP___

#include "oil.hpp"
#include "../inanity/InputStream.hpp"
#include "../inanity/OutputStream.hpp"
#include "../inanity/crypto/crypto.hpp"

BEGIN_INANITY

class File;
class StreamReader;
class StreamWriter;
class MemoryStream;

END_INANITY

BEGIN_INANITY_CRYPTO

class HashStream;

END_INANITY_CRYPTO

BEGIN_INANITY_OIL

class Entity;
class Action;

class FileEntitySchemeInputStream : public InputStream
{
private:
	ptr<Entity> entity;
	bigsize_t totalSize, remainingSize;
	size_t blockSize;
	ptr<StreamReader> descriptorReader;
	ptr<File> totalHashFromDescriptor;
	size_t currentBlockIndex, currentBlockOffset, currentBlockSize;
	ptr<File> currentBlockFile;
	ptr<Crypto::HashStream> hashStream;
	ptr<Crypto::HashStream> totalHashStream;
	bool ended;

public:
	FileEntitySchemeInputStream(ptr<Entity> entity);

	/// Get total size.
	bigsize_t GetSize() const;

	//*** InputStream's methods.
	size_t Read(void* data, size_t size);
};

class FileEntitySchemeOutputStream : public OutputStream
{
private:
	ptr<Action> action;
	ptr<Entity> entity;
	size_t blockSize;
	ptr<Crypto::HashStream> hashStream;
	ptr<Crypto::HashStream> totalHashStream;
	ptr<MemoryStream> descriptorHashStream;
	ptr<StreamWriter> descriptorHashWriter;
	bigsize_t totalSize;
	ptr<MemoryStream> currentBlockStream;
	size_t currentBlockIndex, currentBlockSize;

	void FinishCurrentBlock();

public:
	FileEntitySchemeOutputStream(ptr<Action> action, ptr<Entity> entity, size_t blockSize);

	//*** OutputStream's methods.
	void Write(const void* data, size_t size);

	void End();
};

END_INANITY_OIL

#endif
