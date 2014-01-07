#include "ServerRepo.hpp"
#include "ClientRepo.hpp"
#include "../inanity/MemoryStream.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/FileInputStream.hpp"
#include "../inanity/File.hpp"
#include "../inanity/Strings.hpp"
#include "../inanity/Exception.hpp"
#include <unordered_map>
#include <iostream>
#include <fstream>

BEGIN_INANITY_OIL

class Tester
{
private:
	ptr<ServerRepo> serverRepo;

	struct Client
	{
		ptr<ClientRepo> repo;

		ptr<File> pushFile;
		ptr<File> pullFile;
	};

	typedef std::unordered_map<std::string, Client> Clients;
	Clients clients;

public:

	/// Client commits changes to server.
	void Push(Client& client)
	{
		THROW_ASSERT(!client.pushFile && !client.pullFile);

		ptr<MemoryStream> stream = NEW(MemoryStream());
		StreamWriter writer(stream);

		client.repo->Push(&writer);

		client.pushFile = stream->ToFile();
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
	void Pull(Client& client)
	{
		THROW_ASSERT(!client.pushFile && client.pullFile);

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
		if(value == "null")
			return nullptr;
		return Strings::String2File(value);
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

	void Run(std::istream& in, std::ostream& out)
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
				}
				else if(command == "begin")
				{
					std::string name;
					std::getline(in, name);
					out << "TEST" << name << "\n";

					// do cleanup
					serverRepo = NEW(ServerRepo(":memory:"));
					clients.clear();
				}
				else if(command == "client")
				{
					std::string name;
					in >> name;
					if(clients.find(name) != clients.end())
						THROW("Client with such name already exists: " + name);
					clients[name].repo = NEW(ClientRepo(":memory:"));
				}
				else if(command == "change")
				{
					std::string clientName, key, value;
					in >> clientName >> key >> value;
					GetClient(clientName).repo->Change(Convert(key), Convert(value));
				}
				else if(command == "resolve")
				{
					std::string clientName, key;
					in >> clientName >> key;
					GetClient(clientName).repo->Resolve(Convert(key));
				}
				else if(command == "push")
				{
					std::string clientName;
					in >> clientName;
					Push(GetClient(clientName));
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
					Pull(GetClient(clientName));
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
				else if(command == "check_conflict")
				{
					std::string clientName, key;
					bool conflict;
					in >> clientName >> key >> conflict;
					out << (GetClient(clientName).repo->IsConflicted(Convert(key)) == conflict
						? "OK  " : "FAIL");
					out << " check_conflict " << clientName << " " << key << " " << conflict << "\n";
				}
				else if(command == "check_conflict_server")
				{
					std::string clientName, key, serverValue;
					in >> clientName >> key >> serverValue;
					out << (IsValuesEqual(
						GetClient(clientName).repo->GetConflictServerValue(
							Convert(key)),
						Convert(serverValue))
						? "OK  " : "FAIL");
					out << " check_conflict_server " << clientName << " " << key << " " << serverValue << "\n";
				}
				else if(command == "check_conflict_base")
				{
					std::string clientName, key, baseValue;
					in >> clientName >> key >> baseValue;
					out << (IsValuesEqual(
						GetClient(clientName).repo->GetConflictBaseValue(
							Convert(key)),
						Convert(baseValue))
						? "OK  " : "FAIL");
					out << " check_conflict_base " << clientName << " " << key << " " << baseValue << "\n";
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
		Tester().Run(std::fstream("repo-sync-test.txt", std::ios::in), std::cout);
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
