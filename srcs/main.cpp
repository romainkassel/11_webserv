#include "../incs/WebServ.hpp"
#include "RunServer.hpp"

void	handleSignal( int signum )
{
	if (DEBUG == 1)
	{
		std::cout << std::endl;
		std::cout << "Interrupt signal (" << signum << ") received.\n";
		std::cout << std::endl;
	}

	throw RunServer::RunServerException("Server stopped.");
}

static void	check_arg(int argc, char **argv, std::string &conf_file)
{
	if (argc > 2)
		throw std::invalid_argument("Too many arguments\n" + std::string(WEBSERV_USAGE));
	if (argc == 2)
		conf_file = argv[1];
	else
		conf_file = DEFAULT_CONF_FILE;
}

int	main(int argc, char **argv)
{
	std::cout << std::endl;
	std::cout << "Server starting..." << std::endl;
	std::cout << std::endl;

	/*std::string	request;
	request = "GET / HTTP/1.1\n\rHost: etessier.42.fr\n\rTest: test\n\r\n\rbody test\n\r";
	HttpRequest	httpRequest(request);
	return 0;*/

	std::string	conf_file;

	try
	{
		check_arg(argc, argv, conf_file);
		ConfigParser	parser(conf_file);

		RunServer	runServer(parser.getServers());

		try
		{
			runServer.runThatShit();
		}
		catch (std::exception &e)
		{
			runServer.cleanThatShit();

			std::cerr << std::endl;
			std::cerr << NORMAL << ERROR;
			std::cerr << e.what() << std::endl;
			std::cerr << END;
			std::cerr << std::endl;

			return 1;
		}
	}
	catch (ConfigParser::ConfigParserException &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (ConfigServer::ConfigServerException &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (ConfigLocation::ConfigLocationException &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}