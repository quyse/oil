#include "ServerRepo.hpp"
#include "../inanity/net/Fcgi.hpp"
#include "../inanity/MemoryStream.hpp"
#include "../inanity/StreamReader.hpp"
#include "../inanity/StreamWriter.hpp"
#include "../inanity/File.hpp"
#include "../inanity/Thread.hpp"
#include "../inanity/CriticalSection.hpp"
#include "../inanity/CriticalCode.hpp"
#include "../inanity/Time.hpp"
#include <vector>
#include <iostream>
#include <cstring>

BEGIN_INANITY_OIL

class Server : public Object
{
private:
	ptr<ServerRepo> repo;
	long long lastSyncRevision;
	CriticalSection csRepo;

	std::vector<std::pair<ptr<Net::Fcgi::Request>, Time::Tick> > watchRequests;
	CriticalSection csWatchRequests;

	Server() : lastSyncRevision(-1) {}

	void Process(ptr<Net::Fcgi::Request> request)
	{
		std::cerr << request->GetParam("REQUEST_URI") << '\n';

		StreamReader reader(request->GetInputStream());
		StreamWriter writer(request->GetOutputStream());

		try
		{
			const char* query = request->GetParam("QUERY_STRING");
			if(!query)
				query = "";

			if(strcmp(query, "manifest") == 0)
			{
				request->OutputContentType("application/x-inanityoil-manifest");
				request->OutputStatus("200 OK");
				request->OutputBeginResponse();

				{
					CriticalCode cc(csRepo);
					repo->WriteManifest(&writer);
				}

				request->End();
			}
			else if(strcmp(query, "sync") == 0)
			{
				request->OutputContentType("application/x-inanityoil-sync");
				request->OutputStatus("200 OK");
				request->OutputBeginResponse();

				ptr<File> watchResponse;
				{
					CriticalCode cc(csRepo);
					// TODO: add actual user verification
					// task postponed
					static const String userName = "anonimous";
					repo->Sync(&reader, &writer, userName, true);
					long long maxRevision = repo->GetMaxRevision();
					if(lastSyncRevision < maxRevision)
					{
						ptr<MemoryStream> stream = NEW(MemoryStream());
						StreamWriter writer(stream);
						repo->RespondWatch(&writer);
						watchResponse = stream->ToFile();
					}
					lastSyncRevision = maxRevision;
				}

				request->End();

				if(watchResponse)
				{
					// respond to all watch requests
					CriticalCode cc(csWatchRequests);

					for(size_t i = 0; i < watchRequests.size(); ++i)
						try
						{
							ptr<Net::Fcgi::Request> request = watchRequests[i].first;
							request->GetOutputStream()->WriteFile(watchResponse);
							request->End();
						}
						catch(Exception* exception)
						{
							request->End();
							MakePointer(exception)->PrintStack(std::cerr);
							std::cerr << '\n';
						}

					watchRequests.clear();
				}
			}
			else if(strcmp(query, "watch") == 0)
			{
				request->OutputContentType("application/x-inanityoil-watchreply");
				request->OutputStatus("200 OK");
				request->OutputBeginResponse();

				bool watchResponded;
				{
					CriticalCode cc(csRepo);
					watchResponded = repo->Watch(&reader, &writer);
				}
				if(watchResponded)
					request->End();
				else
				{
					CriticalCode cc(csWatchRequests);
					watchRequests.push_back(std::make_pair(request, Time::GetTicks()));
				}
			}
			else
			{
				// unknown query
				request->OutputContentType("text/plain");
				request->OutputStatus("400 Bad Request");
				request->OutputBeginResponse();
				static const char response[] =
					"Unknown query string";
				writer.Write(response, sizeof(response) - 1);
				request->End();
			}
		}
		catch(Exception* exception)
		{
			request->End();
			MakePointer(exception)->PrintStack(std::cerr);
			std::cerr << '\n';
		}
	}

	void BackgroundProcess(const Thread::ThreadHandler::Result& threadResult)
	{
		const int sleepPause = 1000;
		const Time::Tick timeout = 20 * Time::GetTicksPerSecond();
		for(;;)
		{
			{
				Time::Tick tick = Time::GetTicks();
				CriticalCode cc(csWatchRequests);

				ptr<File> response = nullptr;

				size_t j = 0;
				for(size_t i = 0; i < watchRequests.size(); ++i)
				{
					if(watchRequests[i].second + timeout < tick)
					{
						if(!response)
						{
							ptr<MemoryStream> stream = NEW(MemoryStream());
							StreamWriter writer(stream);
							{
								CriticalCode cc(csRepo);
								repo->RespondWatch(&writer);
							}
							response = stream->ToFile();
						}

						watchRequests[i].first->GetOutputStream()->WriteFile(response);
						watchRequests[i].first->End();
					}
					else
						watchRequests[j++] = watchRequests[i];
				}
				watchRequests.resize(j);
			}

			Thread::Sleep(sleepPause);
		}
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

		MakePointer(NEW(Thread(Thread::ThreadHandler::Bind<Server>(this, &Server::BackgroundProcess))));

		Net::Fcgi fcgi(socketName, backlogSize);

		while(ptr<Net::Fcgi::Request> request = fcgi.Accept())
			Process(request);

		std::cerr << "end\n";

		return 0;
	}

public:
	static int Main(int argc, char** argv)
	{
		try
		{
			return MakePointer(NEW(Server()))->Run(argc, argv);
		}
		catch(Exception* exception)
		{
			MakePointer(exception)->PrintStack(std::cerr);
			std::cerr << "\n";
			return 1;
		}
	}
};

END_INANITY_OIL

int main(int argc, char** argv)
{
	return Inanity::Oil::Server::Main(argc, argv);
}
