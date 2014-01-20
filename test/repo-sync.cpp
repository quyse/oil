#include "../ServerRepo.hpp"
#include "../ClientRepo.hpp"
#include "../../inanity/MemoryStream.hpp"
#include "../../inanity/StreamReader.hpp"
#include "../../inanity/StreamWriter.hpp"
#include "../../inanity/FileInputStream.hpp"
#include "../../inanity/MemoryFile.hpp"
#include "../../inanity/Strings.hpp"
#include "../../inanity/Exception.hpp"
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

BEGIN_INANITY_OIL

class Tester
{
private:
	ptr<ServerRepo> serverRepo;
	int testNumber;
	bool persistentDb;

	struct Client
	{
		std::string name;
		ptr<ClientRepo> repo;

		ptr<File> pushFile;
		ptr<File> pullFile;
	};

	typedef std::unordered_map<std::string, Client> Clients;
	Clients clients;

public:
	Tester(bool persistentDb = false)
	: testNumber(0), persistentDb(persistentDb) {}

	void SendServerManifest(Client& client)
	{
		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);

		serverRepo->WriteManifest(&writer);

		ptr<File> file = stream->ToFile();
		StreamReader reader(NEW(FileInputStream(file)));

		client.repo->ReadServerManifest(&reader);
	}

	/// Client commits changes to server.
	void Push(Client& client, std::ostream& outProtocol)
	{
		THROW_ASSERT(!client.pushFile && !client.pullFile);

		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);

		client.repo->Push(&writer);

		client.pushFile = stream->ToFile();

		outProtocol << "PUSH " << client.name << "\t";
		OutputProtocol(client.pushFile, outProtocol);
	}

	/// Server processes client's commits and respondes back.
	void Sync(Client& client)
	{
		THROW_ASSERT(client.pushFile && !client.pullFile);

		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamReader reader(NEW(FileInputStream(client.pushFile)));
		StreamWriter writer(stream);

		serverRepo->Sync(&reader, &writer);

		client.pushFile = nullptr;
		client.pullFile = stream->ToFile();
	}

	/// Client processes server's response.
	void Pull(Client& client, std::ostream& outProtocol)
	{
		THROW_ASSERT(!client.pushFile && client.pullFile);

		outProtocol << "PULL " << client.name << "\t";
		OutputProtocol(client.pullFile, outProtocol);

		StreamReader reader(NEW(FileInputStream(client.pullFile)));

		client.repo->Pull(&reader);

		client.pullFile = nullptr;
	}

	/// Client cleanups after unsuccessful sync.
	void Cleanup(Client& client)
	{
		THROW_ASSERT(client.pushFile && !client.pullFile);

		client.repo->Cleanup();

		client.pushFile = nullptr;
	}

	Client& GetClient(const std::string& name)
	{
		Clients::iterator i = clients.find(name);
		if(i == clients.end())
			THROW("No client with name " + name);
		return i->second;
	}

	static ptr<File> Convert(const std::string& value)
	{
		BEGIN_TRY();

		if(value == "null")
			return nullptr;

		return Strings::String2File(value);

		END_TRY("Can't convert value");
	}

	void OutputProtocol(ptr<File> file, std::ostream& out)
	{
		const char* data = (const char*)file->GetData();
		size_t size = file->GetSize();
		out << "(" << size << ")\t";
		for(size_t i = 0; i < size; ++i)
		{
			char c = data[i];
			if(c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9')
				out << c;
			else
				out << "\\x" << std::setw(2) << std::setiosflags(std::ios::hex) << std::setfill('0') << (int)c;
		}
		out << "\n";
	}

	static bool IsValuesEqual(ptr<File> a, ptr<File> b)
	{
		if(!a || !b)
			return !a == !b;
		size_t as = a->GetSize();
		size_t bs = b->GetSize();
		if(as != bs)
			return false;
		return memcmp(a->GetData(), b->GetData(), as) == 0;
	}

	void Run(std::istream& in, std::ostream& out, std::ostream& outProtocol)
	{
		in.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);

		try
		{
			for(;;)
			{
				std::string command;
				// ignore eof errors when reading command
				try
				{
					in >> command;
				}
				catch(std::istream::failure error)
				{
					break;
				}

				if(command == "#")
				{
					// skip the rest of the line
					std::string line;
					std::getline(in, line);
					out << "#" << line << "\n";
				}
				else if(command == "begin")
				{
					std::string name;
					std::getline(in, name);
					out << "TEST #" << (++testNumber) << name << "\n";
					outProtocol << "TEST #" << testNumber << name << "\n";

					// do cleanup
					std::string dbName = ":memory:";
					if(persistentDb)
					{
						std::ostringstream ss;
						ss << "test/server_" << testNumber;
						dbName = ss.str();
					}
					serverRepo = NEW(ServerRepo(dbName.c_str()));
					clients.clear();
				}
				else if(command == "client")
				{
					std::string name;
					in >> name;
					if(clients.find(name) != clients.end())
						THROW("Client with such name already exists: " + name);

					std::string dbName = ":memory:";
					if(persistentDb)
					{
						std::ostringstream ss;
						ss << "test/client_" << testNumber << "_" << name;
						dbName = ss.str();
					}
					Client& client = clients[name];
					client.name = name;
					client.repo = NEW(ClientRepo(dbName.c_str()));
					SendServerManifest(client);
				}
				else if(command == "constraints")
				{
					in
						>> serverRepo->maxKeySize
						>> serverRepo->maxValueSize
						>> serverRepo->maxPushKeysCount
						>> serverRepo->maxPushTotalSize
						>> serverRepo->maxPullKeysCount
						>> serverRepo->maxPullTotalSize;
					for(Clients::iterator i = clients.begin(); i != clients.end(); ++i)
						SendServerManifest(i->second);
				}
				else if(command == "change")
				{
					std::string clientName, key, value;
					in >> clientName >> key >> value;
					GetClient(clientName).repo->Change(Convert(key), Convert(value));
				}
				else if(command == "push")
				{
					std::string clientName;
					in >> clientName;
					Push(GetClient(clientName), outProtocol);
				}
				else if(command == "sync")
				{
					std::string clientName;
					in >> clientName;
					Sync(GetClient(clientName));
				}
				else if(command == "pull")
				{
					std::string clientName;
					in >> clientName;
					Pull(GetClient(clientName), outProtocol);
				}
				else if(command == "psp")
				{
					std::string clientName;
					in >> clientName;
					Client& client = GetClient(clientName);
					Push(client, outProtocol);
					Sync(client);
					Pull(client, outProtocol);
				}
				else if(command == "check")
				{
					std::string clientName, key, value;
					in >> clientName >> key >> value;
					out << (IsValuesEqual(
						GetClient(clientName).repo->GetValue(
							Convert(key)),
						Convert(value))
						? "OK  " : "FAIL");
					out << " check " << clientName << " " << key << " " << value << "\n";
				}
				else if(command == "exit")
				{
					out << "EXIT\n";
					break;
				}
				else
					THROW("Unknown command: " + command);
			}
		}
		catch(Exception* exception)
		{
			out << "Inanity Exception:\n  ";
			MakePointer(exception)->PrintStack(out);
			out << "\n";
		}
		catch(std::istream::failure error)
		{
			out << "IO failure:\n  " << error.what() << "\n";
		}

		out << "END\n";
	}
};

void Run()
{
	try
	{
		Tester(false).Run(
			std::fstream("test/repo-sync.txt", std::ios::in),
			std::cout,
			std::fstream("test/repo-sync-protocol.txt", std::ios::out));
	}
	catch(Exception* exception)
	{
		MakePointer(exception)->PrintStack(std::cerr);
	}
}

END_INANITY_OIL

int main()
{
	Inanity::Oil::Run();
	return 0;
}
