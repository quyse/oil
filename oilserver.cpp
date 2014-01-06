#include "ServerRepo.hpp"
#include "../inanity/net/Fcgi.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include <iostream>

BEGIN_INANITY_OIL

class Server : public Handler
{
private:
	Net::Fcgi fcgi;
	ptr<ServerRepo> repo;
	ptr<StreamReader> reader;
	ptr<StreamWriter> writer;

public:
	void Fire()
	{
		std::cerr << "accepted: " << fcgi.GetParam("REQUEST_URI");

		//fcgi.OutputContentType("application/octet-stream");
		fcgi.OutputContentType("text/plain");
		fcgi.OutputStatus("200 OK");
		fcgi.OutputBeginResponse();

		std::cerr << "\n";
	}

	int Run(int argc, char** argv)
	{
		// read options
		argc--;
		argv++;

		const char* fileName = nullptr;
		const char* socketName = ":5000";
		int backlogSize = 10;

		for(int i = 0; i < argc; ++i)
		{
			if(strcmp(argv[i], "-f") == 0)
			{
				if(++i < argc)
					fileName = argv[i];
				else
				{
					std::cerr << "-f requires file name\n";
					return 1;
				}
			}
			else if(strcmp(argv[i], "-s") == 0)
			{
				if(++i < argc)
					socketName = argv[i];
				else
				{
					std::cerr << "-s requires socket name\n";
					return 1;
				}
			}
			else if(strcmp(argv[i], "-b") == 0)
			{
				if(++i < argc)
					backlogSize = atoi(argv[i]);
				else
				{
					std::cerr << "-b requires backlog size\n";
					return 1;
				}
			}
			else
			{
				std::cerr << "Unknown option: " << argv[i] << "\n";
				return 1;
			}
		}

		if(!fileName)
		{
			std::cerr << "You should set db file name with -f\n";
			return 1;
		}

		repo = NEW(ServerRepo(fileName));

		reader = NEW(StreamReader(fcgi.GetInputStream()));
		writer = NEW(StreamWriter(fcgi.GetOutputStream()));

		fcgi.Run(socketName, backlogSize, this);

		std::cerr << "end\n";

		return 0;
	}

	static int Main(int argc, char** argv)
	{
		try
		{
			return MakePointer(NEW(Server()))->Run(argc, argv);
		}
		catch(Exception* exception)
		{
			MakePointer(exception)->PrintStack(std::cerr);
			return 10;
		}
	}
};

END_INANITY_OIL

int main(int argc, char** argv)
{
	return Inanity::Oil::Server::Main(argc, argv);
}
