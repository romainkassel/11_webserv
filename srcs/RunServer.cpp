/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RunServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rkassel <rkassel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 04:54:12 by debian            #+#    #+#             */
/*   Updated: 2025/03/26 15:23:53 by rkassel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RunServer.hpp"
#include "ExecRequest.hpp"

void	handleSignal( int signum );

RunServer::RunServer( void )
{
	if (DEBUG == 1)
		std::cout << "Default constructor (void) from RunServer called" << std::endl;
}

RunServer::RunServer( std::list<ConfigServer> servers ) :
_isServerStarted(false),
_defaultIp("127.0.0.1"),
_serverContainer(servers),
_sockfdClient(-1),
_epfd(-1),
_isBindFinished(false),
_readyFdNb(0),
_addrlen(sizeof(this->_addr)),
_functionName("")
{
	if (DEBUG == 1)
	{
		std::cout << "Default constructor (std::list<ConfigServer> servers) from RunServer called" << std::endl;
		std::cout << std::endl;
	}

	this->setServerIds();

	if (DEBUG == 1)
		this->displayServerContainer(this->_serverContainer);
}

RunServer::RunServer( RunServer const & src )
{
	if (DEBUG == 1)
		std::cout << "Copy constructor from RunServer called" << std::endl;

	*this = src;
}

RunServer::~RunServer( void )
{
	if (DEBUG == 1)
	{
		std::cout << "Destructor from RunServer called" << std::endl;
		std::cout << std::endl;
	}
}

RunServer	&RunServer::operator=( RunServer const & rhs )
{
	if (DEBUG == 1)
		std::cout << "Affectation operator from RunServer called" << std::endl;

	(void)rhs;

	return (*this);
}

void	RunServer::setServerIds( void )
{
	RunServer::displayFunctionTitle("setServerIds");

	std::list<ConfigServer>::iterator	it;

	int i = 0;

	for (it = this->_serverContainer.begin(); it != this->_serverContainer.end(); ++it)
	{
		(*it).set_id(i++);
		(*it).setLocationIds();
	}
}

void	RunServer::runThatShit( void )
{
	RunServer::displayFunctionTitle("runThatShit");

	try
	{
		std::signal(SIGINT, handleSignal);
	}
	catch (std::exception & e)
	{
		throw e;
	}

	this->createEpollInstance();
	this->initSockfdServers();

	while (true)
		this->handleClientConnections();
}

void	RunServer::createEpollInstance( void )
{
	RunServer::displayFunctionTitle("createEpollInstance");

	this->_epfd = epoll_create(1);

	if (this->_epfd == -1)
	{
		this->_functionName = "epoll_create(): ";
		throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
	}
	else
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << SUCCESS;
			std::cout << "Success epoll_create(): epoll instance created through FD " << this->_epfd << "!" << std::endl;
			std::cout << END;

			std::cout << std::endl;
		}
	}
}

void	RunServer::addSockfdToEpollInstance( int const & sockfd )
{
	RunServer::displayFunctionTitle("addSockfdToEpollInstance");

	struct epoll_event	event;

	//event.events = EPOLLIN|EPOLLOUT;
	event.events = EPOLLIN;
	event.data.fd = sockfd;

	if (DEBUG == 1)
		std::cout << "event.events EPOLLIN|EPOLLOUT: " << event.events << std::endl;

	this->_eventContainer.push_back(event);

	if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, sockfd, &(*this->_eventContainer.rbegin())) == -1)
	{
		this->_functionName = "epoll_ctl(ADD): ";
		throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
	}
	else
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << SUCCESS;
			std::cout << "Success EPOLL_CTL_ADD(): FD " << sockfd << " added to epoll_instance " << this->_epfd << " epfd!" << std::endl;
			std::cout << END;
			std::cout << std::endl;

			std::cout << "this->_eventContainer.size(): " << this->_eventContainer.size() << std::endl;
			std::cout << std::endl;
		}
	}
}

void	RunServer::removeSockfdFromEpollInstance( int const & sockfd )
{
	RunServer::displayFunctionTitle("removeSockfdFromEpollInstance");

	if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, sockfd, NULL) == -1)
	{
		this->_functionName = "epoll_ctl(DEL): ";
		throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
	}
	else
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << SUCCESS;
			std::cout << "Success EPOLL_CTL_DEL(): FD " << sockfd << " removed from epoll_instance " << this->_epfd << " epfd!" << std::endl;
			std::cerr << std::endl;
			std::cout << END;
		}
	}
}

void	RunServer::initSockfdServers( void )
{
	RunServer::displayFunctionTitle("initSockfdServers");

	this->getPortsIpAndRecvBufSize();
	this->initAddrinfos();
	this->getSockfdServers();
	this->bindSockfdServersToAddrs();
	this->listenForConnections();
}

void	RunServer::getPortsIpAndRecvBufSize( void )
{
	RunServer::displayFunctionTitle("getPorts");

	std::list<ConfigServer>::iterator	itVS;

	this->_clientBodySizeMax = 0;

	for (itVS = this->_serverContainer.begin(); itVS != this->_serverContainer.end(); ++itVS)
	{
		this->_portContainer.insert((*itVS).get_port());

		if ((*itVS).get_host().compare("0.0.0.0") == 0)
			this->_defaultIp = "0.0.0.0";

		if (DEBUG == 1)
			std::cout << "(*itVS).get_client_body_size(): " << (*itVS).get_client_body_size() << std::endl;

		if ((*itVS).get_client_body_size() > this->_clientBodySizeMax)
			this->_clientBodySizeMax = (*itVS).get_client_body_size();
	}

	if (this->_clientBodySizeMax == 0)
		this->_clientBodySizeMax = DEFAULT_MAX_BODY_SIZE;

	if (DEBUG == 1)
	{
		std::cout << std::endl;
		std::cout << "this->_clientBodySizeMax: " << this->_clientBodySizeMax << std::endl;
		std::cout << std::endl;
	}

	if (DEBUG == 1)
	{
		std::set<uint16_t>::iterator itPC;

		for (itPC = this->_portContainer.begin(); itPC != this->_portContainer.end(); ++itPC)
			std::cout << "*itPC: " << *itPC << std::endl;

		std::cout << std::endl;
	}
}

void	RunServer::initAddrinfos( void )
{
	RunServer::displayFunctionTitle("initAddrinfos");

	std::set<uint16_t>::iterator itPC;
	struct addrinfo				hints;
	int							getaddrinfoCode;
	std::ostringstream			ss;
	std::string					ssString;

	std::memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;

	if (DEBUG == 1)
	{
		std::cout << "this->_defaultIp: " << this->_defaultIp << std::endl;
		std::cout << std::endl;
	}

	for (itPC = this->_portContainer.begin(); itPC != this->_portContainer.end(); ++itPC)
	{
		struct addrinfo*	res;

		ss.str("");
		ss << *itPC;
		ssString = ss.str();

		if (DEBUG == 1)
		{
			std::cout << "this->_defaultIp.c_str(): " << this->_defaultIp.c_str() << std::endl;
			std::cout << "ssString.c_str(): " << ssString.c_str() << std::endl;
			std::cout << std::endl;
		}

		getaddrinfoCode = getaddrinfo(this->_defaultIp.c_str(), ssString.c_str(), &hints, &res);

		if (getaddrinfoCode == 0)
		{
			if (DEBUG == 1)
			{
				std::cout << NORMAL << SUCCESS;
				std::cout << "Success getaddrinfo(): addrinfo structures created!" << std::endl;
				std::cout << END;

				std::cout << std::endl;
			}

			this->_resContainer.push_back(res);
		}
		else
		{
			this->_functionName = "getaddrinfo(): ";
			throw RunServer::RunServerException(this->_functionName + gai_strerror(getaddrinfoCode));
		}
	}

	if (DEBUG == 1)
	{
		std::cout << std::endl;

		std::cout << "this->_resContainer.size(): " << this->_resContainer.size() << std::endl;

		std::cout << std::endl;

		std::vector<struct addrinfo *>::iterator	itRes;

		for (itRes = this->_resContainer.begin(); itRes != this->_resContainer.end(); ++itRes)
		{
			std::cout << "(*itRes)->ai_addr->sa_family: " << (*itRes)->ai_addr->sa_family << std::endl;
			std::cout << "(*itRes)->ai_addr->sa_data: " << (*itRes)->ai_addr->sa_data << std::endl;
			std::cout << std::endl;
		}
	}
}

void	RunServer::getSockfdServers( void )
{
	RunServer::displayFunctionTitle("getSockfdServers");

	if (DEBUG == 1)
	{
		std::cout << "this->_portContainer.size(): " << this->_portContainer.size() << std::endl;
		std::cout << "this->_resContainer.size(): " << this->_resContainer.size() << std::endl;
		std::cout << "this->_serverContainer.size(): " << this->_serverContainer.size() << std::endl;

		std::cout << std::endl;
	}

	int	sockfdServer;

	for (int i = 0; i < static_cast<int>(this->_resContainer.size()); ++i)
	{
		sockfdServer = socket(AF_INET, SOCK_STREAM, 0);

		if (sockfdServer == -1)
		{
			this->_functionName = "socket(): ";
			throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
		}
		else
		{
			if (DEBUG == 1)
			{
				std::cout << NORMAL << SUCCESS;
				std::cout << "Success socket(): Socket server created through FD " << sockfdServer << "!" << std::endl;
				std::cout << END;
				std::cout << std::endl;
			}

			this->addSockfdToEpollInstance(sockfdServer);
			this->_sockfdServersContainer.push_back(sockfdServer);
		}
	}

	if (DEBUG == 1)
	{
		std::cout << std::endl;

		std::cout << "this->_sockfdServersContainer.size(): " << this->_sockfdServersContainer.size() << std::endl;

		std::cout << std::endl;

		std::vector<int>::iterator	itSSC;

		for (itSSC = this->_sockfdServersContainer.begin(); itSSC != this->_sockfdServersContainer.end(); ++itSSC)
			std::cout << "(*itSSC): " << (*itSSC) << std::endl;

		std::cout << std::endl;
	}
}

void	RunServer::bindSockfdServersToAddrs( void )
{
	RunServer::displayFunctionTitle("bindSockfdServersToAddrs");

	std::vector<struct addrinfo *>::iterator	itRes;
	std::vector<int>::iterator					itSSC;

	itSSC = this->_sockfdServersContainer.begin();

	if (DEBUG == 1)
	{
		std::cout << "this->_resContainer.size(): " << this->_resContainer.size() << std::endl;
		std::cout << std::endl;
	}

	for (itRes = this->_resContainer.begin(); itRes != this->_resContainer.end(); ++itRes)
	{
		if (bind(*itSSC, (*itRes)->ai_addr, (*itRes)->ai_addrlen) == -1)
		{
			this->_functionName = "bind(): ";
			throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
		}
		else
		{
			if (DEBUG == 1)
			{
				std::cout << NORMAL << SUCCESS;
				std::cout << "Success bind(): Socket server FD " << *itSSC << " binded!" << std::endl;
				std::cout << END;
			}
		}

		++itSSC;
	}

	this->_isBindFinished = true;

	if (DEBUG == 1)
	{
		std::cout << std::endl;

		std::cout << "this->_isBindFinished: " << this->_isBindFinished << std::endl;
		std::cout << "this->_portContainer.size(): " << this->_portContainer.size() << std::endl;
		std::cout << "this->_resContainer.size(): " << this->_resContainer.size() << std::endl;
		std::cout << "this->_sockfdServersContainer.size(): " << this->_sockfdServersContainer.size() << std::endl;
		std::cout << "this->_serverContainer.size(): " << this->_serverContainer.size() << std::endl;

		std::cout << std::endl;
	}
}

void	RunServer::listenForConnections( void )
{
	RunServer::displayFunctionTitle("listenForConnections");

	std::vector<int>::iterator	itSS;

	for (itSS = this->_sockfdServersContainer.begin(); itSS != this->_sockfdServersContainer.end(); ++itSS)
	{
		if (listen(*itSS, BACKLOG) == -1)
		{
			this->_functionName = "listen(): ";
			throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
		}
		else
		{
			if (DEBUG == 1)
			{
				std::cout << NORMAL << SUCCESS;
				std::cout << "Success listen(): Socket server FD " << *itSS << " is listenning!" << std::endl;
				std::cout << END;
			}
		}
	}

	if (DEBUG == 1)
		std::cout << std::endl;
}

void	RunServer::handleClientConnections( void )
{
	RunServer::displayFunctionTitle("handleClientConnections");

	struct epoll_event events[this->_eventContainer.size()];

	this->_readyFdNb = epoll_wait(this->_epfd, events, 10, 1000);

	if (this->_isServerStarted == false)
	{
		std::cout << NORMAL << SUCCESS;
		std::cout << "Server started." << std::endl;
		std::cout << END;

		std::cout << std::endl;

		this->_isServerStarted = true;
	}

	if (this->_readyFdNb == -1)
	{
		this->_functionName = "epoll_wait(): ";
		throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
	}
	else if (this->_readyFdNb == 0)
	{
		if (DEBUG == 1)
		{
			std::cout << "epoll_wait(): no FD ready for now" << std::endl;
			std::cout << std::endl;
		}
		else
		{
			std::cout << "Waiting for a client connection..." << std::endl;
		}

		return;
	}
	else
	{
		this->handleReadySockfd(events);
	}
}

void	RunServer::handleReadySockfd( struct epoll_event * events )
{
	RunServer::displayFunctionTitle("handleReadySockfd");

	std::vector<int>::iterator	itSS;
	int							isSockfdClient;

	if (DEBUG == 1)
	{
		std::cout << "epoll_wait(): number of FD ready: " << this->_readyFdNb << std::endl;
		std::cout << std::endl;
	}

	for (int i = 0; i < this->_readyFdNb; i++)
	{
		isSockfdClient = false;

		for (itSS = this->_sockfdServersContainer.begin(); itSS != this->_sockfdServersContainer.end(); ++itSS)
		{
			if (DEBUG == 1)
			{
				std::cout << BOLD << WINNER;
				std::cout << "events[i].data.fd: " << events[i].data.fd << std::endl;
				std::cout << "events[i].events: " << events[i].events << std::endl;
				std::cout << END;
			}

			if (events[i].data.fd == *itSS)
			{
				this->getSockfdClient(*itSS);
				this->addSockfdToEpollInstance(this->_sockfdClient);
				isSockfdClient = true;
				break ;
			}
		}

		if (isSockfdClient == false)
		{
			this->_sockfdClient = events[i].data.fd;
			this->readRequestFromClient();
		}
	}
}

void	RunServer::readRequestFromClient( void )
{
	unsigned int	recvByteNbTotal;
	short int		recvByteNb;
	char			buf[RECV_BUFF_SIZE];
	std::string		boundary;
	int				isBodySizeReached;

	recvByteNbTotal = 0;
	recvByteNb = RECV_BUFF_SIZE;
	isBodySizeReached = false;

	this->_clientRequest.clear();

	while (recvByteNb == RECV_BUFF_SIZE)
	{
		buf[0] = '\0';

		recvByteNb = recv(this->_sockfdClient, buf, RECV_BUFF_SIZE, 0);

		recvByteNbTotal += recvByteNb;

		if (recvByteNb > 0)
			buf[recvByteNb] = '\0';

		if (recvByteNb == -1 || recvByteNb == 0)
		{
			std::cerr << "Error recv(): client removed" << std::endl;
			this->handleClosedConnection();
			return ;
		}
		if (isBodySizeReached == true)
		{
			continue;
		}
		else if (recvByteNbTotal > this->_clientBodySizeMax && isBodySizeReached == false)
		{
			if (DEBUG == 1)
			{
				std::cout << "413 Request Entity Too Large" << std::endl;
				std::cout << std::endl;
				std::cout << "recvByteNbTotal: " << recvByteNbTotal << std::endl;
				std::cout << "this->_clientBodySizeMax: " << this->_clientBodySizeMax << std::endl;
				std::cout << std::endl;
			}

			isBodySizeReached = true;
		}
		else
		{
			std::string strBuf = buf;

			if (boundary.size() == 0)
			{
				if (strBuf.find("boundary=") != strBuf.npos)
				{
					// std::cout << "boundary found" << std::endl;
					std::string tmpBuf = buf;
					tmpBuf = tmpBuf.substr(tmpBuf.find("boundary="));
					boundary = "--" + tmpBuf.substr(9, tmpBuf.find('\n') - 10);
					tmpBuf = tmpBuf.substr(tmpBuf.find("\n"));
					// std::cout << "boundary = " << boundary << std::endl;
					strBuf.replace(strBuf.find("boundary=") - 1, 67, "");
					// std::cout << "end boundary strBuf = " << strBuf << std::endl;
				}
			}
			// if boundary found in buf
			if (boundary.size() != 0)
			{
				// std::cout << "boundary set" << std::endl;
				// std::cout << "strBuf.find(boundary) = " << strBuf.find(boundary) << std::endl;
				if (strBuf.find(boundary) != strBuf.npos)
				{
					// std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
					// std::cout << strBuf << std::endl;
					// std::cout << "replace boundary" << std::endl;
					// 	-> remove buf line + previous line
					strBuf.replace(strBuf.find(boundary) - 2, boundary.size() + 4, "");
					// std::cout << strBuf << std::endl;
					// std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
				}
				// std::cout << "(boundary + \"--\") = " << (boundary + "--") << std::endl;
				// std::cout << "strBuf.find(boundary + \"--\") " << strBuf.find(boundary + "--")  << std::endl;
				if (strBuf.find(boundary + "--") != strBuf.npos)
				{
					// std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
					// std::cout << strBuf << std::endl;
					// std::cout << "replace boundary" << std::endl;
					// 	-> remove buf line + previous line
					strBuf.replace(strBuf.find(boundary) - 2, boundary.size() + 6, "");
					// std::cout << strBuf << std::endl;
					// std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
				}
			}
			this->_clientRequest.append(strBuf);
		}
	}

	// if (isBodySizeReached == true)
	// {
	// 	this->sendResponseToClient((*this->_serverContainer.begin()).get_error_pages()[413]);
	// 	return ;
	// }

	if (DEBUG == 1)
	{
		std::cout << "this->_clientRequest: " << std::endl;
		std::cout << this->_clientRequest << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "recvByteNbTotal: " << recvByteNbTotal << std::endl;
		std::cout << "this->_clientRequest.length(): " << this->_clientRequest.length() << std::endl;
		std::cout << std::endl;
	}


	// HTTP PART - START
	if (isBodySizeReached == true)
	{
		HttpRequest	requestTmp(this->_clientRequest, 413);

		this->_request = requestTmp;
	}
	else
	{
		HttpRequest	requestTmp(this->_clientRequest);

		this->_request = requestTmp;
	}
	if (DEBUG == 1)
	{
		if (this->_request.getStatus() > 0)
		{
			if (DEBUG)
				std::cout << "request status = " << this->_request.getStatus() << std::endl;
		}
		else
		{
			this->_request.printStartLine();
			this->_request.printHeaders();
			this->_request.printBody();

			std::cout << "target = " << this->_request.getTarget() << std::endl;
			std::cout << "host = " << this->_request.getHost() << std::endl;
			std::cout << "ip = " << this->_request.getIp() << std::endl;
			std::cout << "port = " << this->_request.getPort() << std::endl;
		}
	}

	if (DEBUG == 1)
	{
		std::cout << "------------------------- Test HttpRequest end -------------------------" << std::endl;
		std::cout << std::endl;
	}

	// HTTP PART - END

	this->handleMessageFromClient();
}

void	RunServer::getSockfdClient( int const & sockfdServer )
{
	RunServer::displayFunctionTitle("getSockfdClient");

	this->_sockfdClient = accept(sockfdServer, &this->_addr, &this->_addrlen);

	if (this->_sockfdClient == -1)
	{
		this->_functionName = "accept(): ";
		throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
	}
	else
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << SUCCESS;
			std::cout << "Success accept(): Socket client accepted and created! " << std::endl;
			std::cout << std::endl;
			std::cout << "FD sockfdClient: " << this->_sockfdClient << std::endl;
			std::cout << "FD sockfdServer: " << sockfdServer << std::endl;
			std::cout << std::endl;
			std::cout << END;
		}
		else
		{
			std::cout << std::endl;
			std::cout << NORMAL << SUCCESS;
			std::cout << "New client connection accepted [";
			std::cout << this->_sockfdClient;
			std::cout << "]."<< std::endl;
			std::cout << END;
		}
	}
}

void	RunServer::handleClosedConnection( void )
{
	RunServer::displayFunctionTitle("handleClosedConnection");

	this->removeSockfdFromEpollInstance(this->_sockfdClient);
	this->shutdownSockfd(this->_sockfdClient);
	this->closeFd(this->_sockfdClient);
	this->removeEventFromContainer(this->_sockfdClient);

	if (DEBUG == 1)
	{
		std::cout << "recv(): client connection closed" << std::endl;
		std::cout << std::endl;
	}
	else
	{
		std::cout << NORMAL << ERROR;
		std::cout << "Client connection closed [";
		std::cout << this->_sockfdClient;
		std::cout << "]."<< std::endl;
		std::cout << END;
		std::cout << std::endl;
	}
}

void	RunServer::shutdownSockfd( int const & sockfd )
{
	RunServer::displayFunctionTitle("shutdownSockfd");

	if (shutdown(sockfd, SHUT_RDWR) == -1)
	{
		if (DEBUG == 1)
			std::cerr << "sockfd:" << sockfd << std::endl;

		this->_functionName = "shutdown(): ";
		throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
	}
	else
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << SUCCESS;
			std::cout << "Success shutdown(): sockfd " << sockfd << " shutted down!" << std::endl;
			std::cout << std::endl;
			std::cout << END;
		}
	}
}

void	RunServer::closeFd( int const & fd )
{
	RunServer::displayFunctionTitle("closeFd");

	if (close(fd) == -1)
	{
		this->_functionName = "close(): ";
		throw RunServer::RunServerException(this->_functionName + std::strerror(errno));
	}
	else
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << SUCCESS;
			std::cout << "Success close(): fd " << fd << " closed!" << std::endl;
			std::cout << std::endl;
			std::cout << END;
		}
	}
}

void	RunServer::removeEventFromContainer( int const & sockfd )
{
	RunServer::displayFunctionTitle("removeEventFromContainer");

	std::vector<struct epoll_event>::iterator	it;

	for (it = this->_eventContainer.begin(); it != this->_eventContainer.end(); ++it)
	{
		if ((*it).data.fd == sockfd)
		{
			if (DEBUG == 1)
				std::cout << "(*it).data.fd: " << (*it).data.fd << std::endl;
			this->_eventContainer.erase(it);
			break;
		}
	}
}

void	RunServer::handleMessageFromClient( void )
{
	RunServer::displayFunctionTitle("handleMessageFromClient");

	if (DEBUG == 1)
	{
		std::cout << NORMAL << SUCCESS;
		std::cout << "Success recv(): data read and buffered from sockfd_client " << this->_sockfdClient << "!" << std::endl;
		std::cout << END;

		std::cout << std::endl;

		std::cout << "RECV_BUFF_SIZE: " << RECV_BUFF_SIZE << std::endl;

		std::cout << std::endl;
	}

	if (DEBUG == 1)
		std::cout << "------------------------- Test HttpRequest start -------------------------" << std::endl;

	if (this->_request.getStatus() == 0)
	{
		this->getMatchingVirtualServer();
		this->getMatchingLocationBlock();
		this->sendResponseToClient(this->_winnerServer->get_file_to_return());
	}
	else
		this->sendResponseToClient("");

}

void	RunServer::getMatchingVirtualServer( void )
{
	RunServer::displayFunctionTitle("getMatchingVirtualServer");

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainer.size(): " << this->_serverContainer.size() << std::endl;
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << std::endl;
	}

	this->_serverContainerTmp.clear();
	this->_winnerServer = NULL;

	if (this->_serverContainer.size() == 1)
		this->_winnerServer = &(*this->_serverContainer.begin());
	else
	{
		this->getVSMatchingHostPort();

		if (this->_serverContainerTmp.size() > 1)
			this->getSpecificListenMatches();
		if (this->_serverContainerTmp.size() > 1)
			this->getExactServerNameMatches();
		if (this->_serverContainerTmp.size() > 1)
			this->getLeadingWildcardMatches();
		if (this->_serverContainerTmp.size() > 1)
			this->getTrailingWildcardMatches();
		if (this->_serverContainerTmp.size() > 1)
			this->getFirstVirtualServer();

		this->_winnerServer = &(*this->_serverContainerTmp.begin());
	}

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << std::endl;

		if (this->_serverContainerTmp.size() == 1 || this->_winnerServer != NULL)
		{
			std::cout << BOLD << WINNER;
			std::cout << "Winning and matching VirtualServer is: " << std::endl;
			std::cout << END;
			std::cout << std::endl;
			this->_winnerServer->display_config();
			std::cout << std::endl;
		}
		else
		{
			throw RunServer::RunServerException("Still no virtual server selected (at least 2 in competition).");
		}
	}
}

void	RunServer::getVSMatchingHostPort( void )
{
	RunServer::displayFunctionTitle("getVSMatchingHostPort");

	std::list<ConfigServer>::iterator	it;
	//std::string						tmpPort;

	for (it = this->_serverContainer.begin(); it != this->_serverContainer.end(); ++it)
	{
		if (DEBUG == 1)
		{
			std::cout << "(*it).get_port(): " << (*it).get_port() << std::endl;
			std::cout << "this->_request.getPort(): " << this->_request.getPort() << std::endl;
		}

		/*tmpPort = this->_request.get_port().substr(0, 4);

		if (DEBUG == 1)
			std::cout << "tmpPort: " << tmpPort << std::endl;*/

		if ((*it).get_port() == this->_request.getPort())
		{
			if (DEBUG == 1)
				std::cout << "condition!!" << std::endl;
			this->_serverContainerTmp.push_back(*it);
		}

		if (DEBUG == 1)
			std::cout << std::endl;
	}

	//this->displayServerContainer(this->_serverContainerTmp);
}

void	RunServer::getSpecificListenMatches( void )
{
	RunServer::displayFunctionTitle("getSpecificListenMatches");

	std::list<ConfigServer>				serverContainerTmpCpy;
	std::list<ConfigServer>::iterator	it;

	serverContainerTmpCpy = this->_serverContainerTmp;

	for (it = this->_serverContainerTmp.begin(); it != this->_serverContainerTmp.end(); ++it)
	{
		if (DEBUG == 1)
		{
			std::cout << "(*it).get_host(): " << (*it).get_host() << std::endl;
			std::cout << "this->_request.getIp(): " << this->_request.getIp() << std::endl;
			std::cout << std::endl;
		}

		if ((*it).get_host().compare(this->_request.getIp()) != 0)
			this->_serverContainerTmp.erase(it--);
	}

	if (this->_serverContainerTmp.size() == 0)
		this->_serverContainerTmp = serverContainerTmpCpy;

	//this->displayServerContainer(this->_serverContainerTmp);
}

void	RunServer::getExactServerNameMatches( void )
{
	RunServer::displayFunctionTitle("getExactServerNameMatches");

	std::list<ConfigServer>				serverContainerTmpCpy;
	std::list<ConfigServer>::iterator	it;

	serverContainerTmpCpy = this->_serverContainerTmp;

	for (it = this->_serverContainerTmp.begin(); it != this->_serverContainerTmp.end(); ++it)
	{
		if (DEBUG == 1)
		{
			std::cout << std::endl;
			std::cout << "(*it).get_server_name(): " << (*it).get_server_name() << std::endl;
			std::cout << "this->_request.getHost(): " << this->_request.getHost() << std::endl;
		}

		if ((*it).get_server_name().compare(this->_request.getHost()) != 0)
			this->_serverContainerTmp.erase(it--);
	}

	if (this->_serverContainerTmp.size() > 1)
		this->getFirstVirtualServer();

	if (this->_serverContainerTmp.size() == 0)
		this->_serverContainerTmp = serverContainerTmpCpy;

	//this->displayServerContainer(this->_serverContainerTmp);
}

void	RunServer::getLeadingWildcardMatches( void )
{
	RunServer::displayFunctionTitle("getLeadingWildcardMatches");

	std::list<ConfigServer>				serverContainerTmpCpy;
	std::list<ConfigServer>::iterator	it;
	std::string							tmp;
	int									start;
	long unsigned int					longestMatch;

	serverContainerTmpCpy = this->_serverContainerTmp;
	longestMatch = 0;

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << std::endl;
	}

	for (it = this->_serverContainerTmp.begin(); it != this->_serverContainerTmp.end(); ++it)
	{
		if (DEBUG == 1)
			std::cout << "this->_request.getHost(): " << this->_request.getHost() << std::endl;

		tmp = "";

		if ((*it).get_server_name()[0] == '*')
		{
			start = this->_request.getHost().length() - (*it).get_server_name().length();

			tmp = this->_request.getHost().substr(start);
			tmp[0] = '*';

			if (DEBUG == 1)
			{
				std::cout << "Leading wildcard !!!!!" << std::endl;
				std::cout << "tmp                  : " << tmp << std::endl;
				std::cout << "(*it).get_server_name(): " << (*it).get_server_name() << std::endl;
			}
		}

		if (DEBUG == 1)
			std::cout << std::endl;

		if (tmp.compare((*it).get_server_name()) != 0)
			this->_serverContainerTmp.erase(it--);
		else if (tmp.length() > longestMatch)
			longestMatch = tmp.length();
	}

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << "longestMatch: " << longestMatch << std::endl;
		std::cout << std::endl;
	}

	//this->displayServerContainer(this->_serverContainerTmp);

	if (this->_serverContainerTmp.size() > 1)
		this->removeNonLongestMatches(longestMatch);

	if (this->_serverContainerTmp.size() == 0)
		this->_serverContainerTmp = serverContainerTmpCpy;

	//this->displayServerContainer(this->_serverContainerTmp);
}

void	RunServer::removeNonLongestMatches( long unsigned int const & longestMatch )
{
	RunServer::displayFunctionTitle("removeNonLongestMatches");

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << std::endl;
	}

	std::list<ConfigServer>::iterator	it;

	for (it = this->_serverContainerTmp.begin(); it != this->_serverContainerTmp.end(); ++it)
		if ((*it).get_server_name().length() < longestMatch)
			this->_serverContainerTmp.erase(it--);

	//this->displayServerContainer(this->_serverContainerTmp);

	if (this->_serverContainerTmp.size() > 1)
		this->getFirstVirtualServer();

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << std::endl;
	}
}

void	RunServer::getTrailingWildcardMatches( void )
{
	RunServer::displayFunctionTitle("getTrailingWildcardMatches");

	std::list<ConfigServer>				serverContainerTmpCpy;
	std::list<ConfigServer>::iterator	it;
	std::string							tmp;
	int									end;
	long unsigned int					longestMatch;

	serverContainerTmpCpy = this->_serverContainerTmp;
	longestMatch = 0;

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << std::endl;
	}

	for (it = this->_serverContainerTmp.begin(); it != this->_serverContainerTmp.end(); ++it)
	{
		if (DEBUG == 1)
		{
			std::cout << "this->_request.getHost(): " << this->_request.getHost() << std::endl;
			std::cout << "(*it).get_server_name(): " << (*it).get_server_name() << std::endl;
		}

		tmp = "";
		end = (*it).get_server_name().length() - 1;

		if ((*it).get_server_name()[end] == '*')
		{
			if (DEBUG == 1)
				std::cout << "Trailing wildcard !!!!!" << std::endl;

			tmp = this->_request.getHost().substr(0, end + 1);
			tmp[end] = '*';

			if (DEBUG == 1)
			{
				std::cout << "tmp                    : " << tmp << std::endl;
				std::cout << "(*it).get_server_name(): " << (*it).get_server_name() << std::endl;
			}
		}

		if (DEBUG == 1)
			std::cout << std::endl;

		if (tmp.compare((*it).get_server_name()) != 0)
			this->_serverContainerTmp.erase(it--);
		else if (tmp.length() > longestMatch)
			longestMatch = tmp.length();
	}

	if (DEBUG == 1)
	{
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << "longestMatch: " << longestMatch << std::endl;
		std::cout << std::endl;
	}

	//this->displayServerContainer(this->_serverContainerTmp);

	if (this->_serverContainerTmp.size() > 1)
		this->removeNonLongestMatches(longestMatch);

	if (this->_serverContainerTmp.size() == 0)
		this->_serverContainerTmp = serverContainerTmpCpy;

	//this->displayServerContainer(this->_serverContainerTmp);
}

void	RunServer::getFirstVirtualServer( void )
{
	RunServer::displayFunctionTitle("getFirstVirtualServer");

	if (DEBUG == 1)
	{
		std::cout << std::endl;
		std::cout << "this->_serverContainerTmp.size(): " << this->_serverContainerTmp.size() << std::endl;
		std::cout << std::endl;
	}

	std::list<ConfigServer>::iterator	it1;
	std::list<ConfigServer>::iterator	it2;

	it1 = this->_serverContainerTmp.begin();
	it1++;

	it2 = this->_serverContainerTmp.end();

	this->_serverContainerTmp.erase(it1, it2);
}

void	RunServer::getMatchingLocationBlock( void )
{
	const std::string&	target = this->_request.getTarget();

	if (DEBUG == 1)
	{
		std::cout << "this->_request.getTarget(): " << this->_request.getTarget() << std::endl;
		std::cout << std::endl;
	}

	this->_winnerServer->set_target(target);
	this->_winnerServer->getMatchingConfigLocation();
}

void	RunServer::sendResponseToClient( std::string const & fileToReturn )
{
	RunServer::displayFunctionTitle("sendResponseToClient");

	HttpResponse responseTmp;
	this->_response = responseTmp;
	if (DEBUG)
		std::cout << "this->_request.getStatus() : " << this->_request.getStatus() << std::endl;
	if (this->_request.getStatus() == 400)
		this->_response.setBadRequest();
	else
	{
		this->_response.setFileToReturn(fileToReturn);
		this->_request.setWinnerServer(this->_winnerServer);
		execRequest(this->_request, this->_response);
	}

	short int	sendByteNb;

	sendByteNb = 0;

	sendByteNb = send(this->_sockfdClient, this->_response.build().c_str(), this->_response.build().length(), 0);

	if (sendByteNb == -1 || sendByteNb < static_cast<short int>(this->_response.build().length()))
	{
		std::cerr << "Error send(): client removed" << std::endl;
		//this->handleClosedConnection();
		return ;
	}
	else if (sendByteNb == static_cast<short int>(this->_response.build().size()))
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << SUCCESS;
			std::cout << "Success send(): response fully sent to sockfd_client " << this->_sockfdClient << "!" << std::endl;
			std::cout << END;

			std::cout << std::endl;

			std::cout << "Data sent: " << std::endl;
			std::cout << this->_response.build() << std::endl;
			std::cout << "this->_response.build().size(): " << this->_response.build().size() << std::endl;
			std::cout << "Number of bytes sent: " << sendByteNb << std::endl;

			std::cout << std::endl;
		}
	}
	else
	{
		if (DEBUG == 1)
		{
			std::cout << NORMAL << ERROR;
			std::cout << "Error send(): response not fully sent to sockfd_client " << this->_sockfdClient << "!" << std::endl;
			std::cout << END;

			std::cout << std::endl;

			std::cout << "this->_response.build().size(): " << this->_response.build().size() << std::endl;
			std::cout << "Number of bytes sent: " << sendByteNb << std::endl;

			std::cout << std::endl;
		}
	}
}

void	RunServer::cleanThatShit( void )
{
	RunServer::displayFunctionTitle("cleanThatShit");

	this->freeAddrinfos();
	this->cleanRemainingSockfdS();
	this->closeFd(this->_epfd);
}

void	RunServer::freeAddrinfos( void )
{
	RunServer::displayFunctionTitle("freeAddrinfos");

	std::vector<struct addrinfo *>::iterator	itRes;

	for (itRes = this->_resContainer.begin(); itRes != this->_resContainer.end(); ++itRes)
		freeaddrinfo(*itRes);
}

void	RunServer::cleanRemainingSockfdS( void )
{
	RunServer::displayFunctionTitle("cleanRemainingSockfdS");

	std::vector<struct epoll_event>::iterator	it;

	if (DEBUG == 1)
		std::cout << "this->_isBindFinished: " << this->_isBindFinished << std::endl;

	for (it = this->_eventContainer.begin(); it != this->_eventContainer.end(); ++it)
	{
		if (DEBUG == 1)
			std::cout << "(*it).data.fd: " << (*it).data.fd << std::endl;

		if (this->_isBindFinished == true)
			shutdownSockfd((*it).data.fd);
		closeFd((*it).data.fd);
	}
}

// Exceptions

RunServer::RunServerException::RunServerException( std::string _errorMessage ) throw() :
_errorMessage(_errorMessage)
{
	if (DEBUG == 1)
	{
		std::cout << "Default constructor (std::string _errorMessage) from RunServerException called" << std::endl;
		std::cout << std::endl;
	}
}

const char *RunServer::RunServerException::what() const throw()
{
	return (this->_errorMessage.c_str());
}

RunServer::RunServerException::~RunServerException( void ) throw()
{
	if (DEBUG == 1)
	{
		std::cout << "Default destructor from RunServerException called" << std::endl;
		std::cout << std::endl;
	}
}

std::string	RunServer::removeMultipleSlashs( std::string fileToReturn )
{
	std::size_t	found = fileToReturn.find("//");

	while (found != std::string::npos)
	{
		fileToReturn.replace(found,2,"/");
		found = fileToReturn.find("//");
	}

	return (fileToReturn);
}


// For tests

void	RunServer::displayFunctionTitle( std::string const & functionTitle )
{
	if (DEBUG == 1)
	{
		std::cout << BOLD << FUNCTION;
		std::cout << "Function: " << functionTitle << std::endl << std::endl;
		std::cout << END;
	}
}

void	RunServer::displayServerContainer( std::list<ConfigServer> const & serverContainer ) const
{
	if (DEBUG == 1)
	{
		RunServer::displayFunctionTitle("displayServerContainer");

		std::list<ConfigServer>::const_iterator	it;

		std::cout << "serverContainer.size(): " << serverContainer.size() << std::endl;

		std::cout << std::endl;

		for (it = serverContainer.begin(); it != serverContainer.end(); ++it)
		{
			(*it).display_config();
			std::cout << std::endl;
			(*it).displayLocationContainer("");
			std::cout << std::endl;
		}
	}
}