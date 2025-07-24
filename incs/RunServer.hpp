/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RunServer.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rkassel <rkassel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 04:54:12 by debian            #+#    #+#             */
/*   Updated: 2025/03/05 17:37:02 by rkassel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __RUNSERVER_HPP_
#define __RUNSERVER_HPP_

//En-tete C a verifier (existe en c++ ?)
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>

//En-tete C++ SUR
#include <cerrno>
#include <cstring>
#include <vector>
#include <set>
#include <list>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <algorithm>
#include <sstream>

#include "HttpRequest.hpp"
#include "ConfigServer.hpp"

#define BACKLOG 125
#define RECV_BUFF_SIZE 1024
//#define RECV_BUFF_SIZE 8192
#define SEND_BUFF_SIZE 50
#define DEFAULT_MAX_BODY_SIZE 1024
#define DEFAULT_PORT 8080
#define DEBUG 0

// start colors

#define BOLD "\033[1;"
#define NORMAL "\033[0;"

#define WHITE "37m" // blanc
#define ERROR "31m" // red
#define SUCCESS "32m" // green
#define WINNER "36m" // Cyan
#define FUNCTION "33m" // yellow

#define END "\033[0m"

// end colors

class RunServer {

private:

	bool							_isServerStarted;

	std::string						_defaultIp;

	std::list<ConfigServer>			_serverContainer;
//	std::map<std::string, std::vector<ConfigServer * > >		_serverContainer;//_serverContainer["127.0.0.1:8080"]
	std::set<uint16_t>				_portContainer;
	std::vector<int>				_sockfdServersContainer;
	std::vector<struct addrinfo *>	_resContainer;

	int	_sockfdClient;

	int								_epfd;
	std::vector<struct epoll_event>	_eventContainer;
	int								_isBindFinished;
	int								_readyFdNb;

	struct sockaddr					_addr;
	socklen_t 						_addrlen;

	unsigned int					_clientBodySizeMax;

	std::string						_clientRequest;

	std::list<ConfigServer>			_serverContainerTmp;
	ConfigServer*					_winnerServer;
	HttpRequest						_request;
	HttpResponse					_response;

	std::string						_functionName;

	RunServer( void );

public:

	RunServer( std::list<ConfigServer>	servers );
	RunServer( RunServer const & src );
	~RunServer( void );

	RunServer	&operator=( RunServer const & rhs );

	void	setServerIds( void );

	void	runThatShit( void );

	void	createEpollInstance( void );
	void	addSockfdToEpollInstance( int const & sockfd );
	void	removeSockfdFromEpollInstance( int const & sockfd );

	void	handleClosedConnection( void );
	void	shutdownSockfd( int const & sockfd );
	void	closeFd( int const & fd );
	void	removeEventFromContainer( int const & sockfd );

	void	initSockfdServers( void );
	void	getPortsIpAndRecvBufSize( void );
	void	initAddrinfos( void );
	void	getSockfdServers( void );
	void	bindSockfdServersToAddrs( void );
	void	listenForConnections( void );

	void	handleClientConnections( void );
	void	handleReadySockfd( struct epoll_event * events );
	void	readRequestFromClient( void );
	void	getSockfdClient( int const & sockfdServer );
	void	handleMessageFromClient( void );

	void	getMatchingVirtualServer( void );
	void	getVSMatchingHostPort( void );
	void	getSpecificListenMatches( void );
	void	getExactServerNameMatches( void );
	void	getLeadingWildcardMatches( void );
	void	getTrailingWildcardMatches( void );
	void	removeNonLongestMatches( long unsigned int const & longestMatch );
	void	getFirstVirtualServer( void );

	void	getMatchingLocationBlock( void );

	void	sendResponseToClient( std::string const & fileToReturn );

	void	cleanThatShit( void );
	void	freeAddrinfos( void );
	void	cleanRemainingSockfdS( void );

	static std::string	removeMultipleSlashs( std::string fileToReturn );

	// For tests

	static void	displayFunctionTitle( std::string const & functionTitle );
	void		displayServerContainer( std::list<ConfigServer> const & serverContainer ) const;

	class RunServerException : public std::exception {

		private:

			std::string	_errorMessage;

		public:

			RunServerException( std::string _errorMessage ) throw();
			virtual const char *what() const throw();
			~RunServerException( void ) throw();

	};
};

#endif
