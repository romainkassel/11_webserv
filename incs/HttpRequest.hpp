#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <map>
# include <algorithm>
# include <fstream>
# include "HttpResponse.hpp"
# include <stdint.h>
# include <sstream>

# include "ConfigServer.hpp"

# define REQUEST_EOL "\r\n"

typedef std::map<std::string, std::string>::iterator mapIt;
typedef std::map<std::string, std::string>::const_iterator mapItConst;

class HttpRequest
{
	private:
		std::string		_method;
		std::string		_target;
		std::string		_protocol;

		std::map<std::string, std::string>	_headers;

		std::string		_body;

		std::string		_host;
		std::string		_ip;
		std::string		_port;

		ConfigServer*	_winnerServer;

		int				_status;

	public:
		HttpRequest(void);
		HttpRequest( HttpRequest const & src );
		HttpRequest( std::string, int );
		HttpRequest(std::string);
		~HttpRequest(void);

		HttpRequest	&operator=( HttpRequest const & rhs );

		void		setStartLine(std::string &);
		void		setHeaders(std::string &);
		void		setBody(std::string &);
		void		setHostData(void);
		void		setStatus(int);
		void		setWinnerServer(ConfigServer *);
		void		getIpFromHost(void);

		std::string		getMethod(void) const;
		std::string		getTarget(void) const;
		std::string		getProtocol(void) const;
		std::string		getHeader(const char *);
		std::string		getBody(void) const;
		std::string		getIp(void) const;
		std::string		getHost(void) const;
		uint16_t		getPort(void) const;
		int				getStatus(void) const;
		ConfigServer*	getWinnerServer(void) const;

		void		printStartLine(void) const;
		void		printHeaders(void) const;
		void		printBody(void) const;

		void		checkRequest(void);

		class HeaderNotFoundException : public std::exception
		{
		public:
			virtual const char* what() const throw()
			{
				return ("Error: Header not found.");
			}
		};

		class UnknownMethodException : public std::exception
		{
		public:
			virtual const char* what() const throw()
			{
				return ("Error: Unknown method.");
			}
		};

		class UnknownProtocolException : public std::exception
		{
		public:
			virtual const char* what() const throw()
			{
				return ("Error: Unknown protocol.");
			}
		};
	};

std::string		getLineRequest(std::string &);

bool			isIpAddress(std::string);

#endif

/*
	Request format :
	Start line (mandatory)				<METHOD> <TARGET> <PROTOCOL>
	Request headers (optional)			<HEADER>: <VALUE(S)>
										...
	Representation headers (optional)	<HEADER>: <VALUE(S)>;<VALUE(S)>
		(only for POST request)			...
										<EMPTY_LINE>
	Response body (optional)			<DATA>
		(only for POST request)			...


	Methods allowed :
		- GET
		- POST
		- DELETE

	Target types :
		- Origin form :	relative path of the target used with 'Host' header to have absolute path,
						used for GET and POST methods (ex : /en-US/docs/Web/HTTP/Messages)
		- Absolute form :	absolute path of the target, used with GET method
							(ex : https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages)

	Headers :
		- Request header : used only for requests to provide more informations and context.
		- Representation header :	used only for requests with body (POST method), describes how to
									interpret the data (format, encoding, length, etc...)

	Body :
		Data sent with POST method, described in the representation headers
*/
