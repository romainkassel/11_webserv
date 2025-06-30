#include "../incs/HttpRequest.hpp"
#include "RunServer.hpp"

HttpRequest::HttpRequest(void)
{
}

//	Takes the request in a std::string as parameter and parses it.
HttpRequest::HttpRequest(std::string raw_request) : _host(""), _ip(""), _port(""), _status(0)
{
	setStartLine(raw_request);
	if (this->getStatus() > 0)
		return;
	setHeaders(raw_request);
	setBody(raw_request);
	setHostData();
	if (this->getStatus() > 0)
		return;
	checkRequest();
}
// HttpRequest::HttpRequest(std::string raw_request, int error
HttpRequest::HttpRequest( std::string request, int status ) : _host(""), _ip(""), _port(""), _status(status)
{
	if (DEBUG)
		std::cout << "start error, status = " << status << std::endl;
	this->_method = "GET";
	this->_target = "/";
	this->_protocol = "HTTP/1.1";

	int len = request.find(REQUEST_EOL);
	request = request.substr(len + 2);

	setHeaders(request);
	setHostData();
	checkRequest();
	if (DEBUG)
	{
		printStartLine();
		printHeaders();
		std::cout << "end error" << std::endl;
	}
}

HttpRequest::HttpRequest( HttpRequest const & src )
{
	*this = src;
}

HttpRequest::~HttpRequest(void)
{
	if (DEBUG == 1)
	{
		std::cout << "Destructor from HttpRequest called" << std::endl;
		std::cout << std::endl;
	}

}

HttpRequest	&HttpRequest::operator=( HttpRequest const & rhs )
{
	this->_method = rhs._method;
	this->_target = rhs._target;
	this->_protocol = rhs._protocol;
	this->_headers = rhs._headers;
	this->_body = rhs._body;
	this->_host = rhs._host;
	this->_ip = rhs._ip;
	this->_port = rhs._port;
	this->_status = rhs._status;

	return (*this);
}

//	Takes the request as parameter, parses the first line (start line) in 3 :
//	The method, the target and the protocol.
//	<METHOD> <TARGET> <PROTOCOL>
void		HttpRequest::setStartLine(std::string &line)
{
	int len = line.find(" ");
	if (len < 0)
	{
		this->setStatus(400);
		return ;
	}
	std::string part = line.substr(0, len);
	if (DEBUG)
	{
		std::cout << "setStartLine line : " << line << std::endl;
		std::cout << "line.find(\"delete.cgi\") = " << line.find("delete.cgi") << std::endl;
	}
	if (line.find("delete.cgi") != line.npos && line.find("delete.cgi") < line.find("\n"))
		part = "DELETE";
	line = line.substr(len + 1);
	this->_method = part;

	len = line.find(" ");
	part = line.substr(0, len);
	line = line.substr(len + 1);
	this->_target = part;

	if (DEBUG == 1)
	{
		std::cout << "part: " << part << std::endl;
		std::cout << "this->_target: " << this->_target << std::endl;
		std::cout << "line: " << line << std::endl;
	}

	len = line.find(REQUEST_EOL);
	part = line.substr(0, len);
	line = line.substr(len + 2);
	this->_protocol = part;
}

	//	Takes the request without the startline as parameter, parses the headers in a map name, value.
//	<NAME>: <VALUE>
void		HttpRequest::setHeaders(std::string &request)
{
	std::string	line = getLineRequest(request);
	while (!request.empty() && !line.empty())
	{
		// std::cout << "header line = " << line << std::endl;
		int len_key = line.find(":");
		std::string key = line.substr(0, len_key);
		line = line.substr(len_key + 2);
		std::string value = line.substr(0, line.find(REQUEST_EOL));
		this->_headers.insert(std::pair<std::string, std::string>(key, value));
		line = getLineRequest(request);
	}
}

//	Takes the request without the startline and headers as parameters, if the request is
//	not empty it means there is a body.
//	<BODY>
void			HttpRequest::setBody(std::string &request)
{
	if (!request.empty())
		this->_body = request;
}

void			HttpRequest::setHostData(void)
{
	std::string	host_header = this->getHeader("Host");
	if (host_header.empty())
		return;

	if (host_header.find(":") != std::string::npos)
		this->_port = host_header.substr(host_header.find(":") + 1, host_header.find("\n"));

	if (DEBUG == 1)
		std::cout << "this->_port: " << this->_port << std::endl;

	std::string	tmp_host = host_header.substr(0, host_header.find(":"));

	if (isIpAddress(tmp_host))
	{
		this->_ip = tmp_host;
		this->_host = "";
	}
	else
	{
		this->_host = tmp_host;
		this->getIpFromHost();
	}
}

void			HttpRequest::setStatus(int status)
{
	this->_status = status;
}

void			HttpRequest::setWinnerServer(ConfigServer *winnerServer)
{
	this->_winnerServer = winnerServer;
	//this->_winnerServer->display_config();
}

void			HttpRequest::getIpFromHost(void)
{
	std::ifstream	ifs("/etc/hosts");
	std::string		ifsContent;
	std::size_t		pos;

	// while (getline(ifs, ifsContent) != NULL)
	while (getline(ifs, ifsContent))
	{
		pos = ifsContent.find(this->_host);

		if (pos == std::string::npos)
			continue;
		else
		{
			pos = ifsContent.find(' ');
			ifsContent = ifsContent.substr(0, pos);
			this->_ip = ifsContent;
			break;
		}
	}

	if (this->_ip.compare("") == 0)
		this->_ip = "127.0.0.1";

	ifs.close();
}

std::string		HttpRequest::getMethod(void) const
{
	return (this->_method);
}

std::string		HttpRequest::getTarget(void) const
{
	return (this->_target);
}

std::string		HttpRequest::getProtocol(void) const
{
	return (this->_protocol);
}

//	Takes the name of the header searched as parameter and returns its value.
//	If the header is not in the request an HttpRequest::HeaderNotFoundException() is thrown.
std::string		HttpRequest::getHeader(const char *header)
{
	mapItConst	it = this->_headers.find(header);
	if (it == this->_headers.end())
	{
		std::cout << "header not found : " << header << std::endl;
		this->_status = 400;
		return ("");
	}
	return ((*it).second);
}

std::string		HttpRequest::getBody(void) const
{
	return (this->_body);
}

std::string		HttpRequest::getIp(void) const
{
	return (this->_ip);
}
std::string		HttpRequest::getHost(void) const
{
	return (this->_host);
}
uint16_t		HttpRequest::getPort(void) const
{
	std::stringstream	ss;
	uint16_t			port;

	ss << this->_port;
	ss >> port;

	return (port);
}

int				HttpRequest::getStatus(void) const
{
	return (this->_status);
}

ConfigServer*	HttpRequest::getWinnerServer(void) const
{
	return (this->_winnerServer);
}

void			HttpRequest::printStartLine(void) const
{
	std::cout << "start line :" << this->getMethod() << " " << this->getTarget() << " " << this->getProtocol() << std::endl;
}

void			HttpRequest::printHeaders(void) const
{
	mapItConst ite = this->_headers.end();
	for (mapItConst it = this->_headers.begin(); it != ite; it++)
		std::cout << (*it).first << ": " << (*it).second << std::endl;
}

void			HttpRequest::printBody(void) const
{
	std::cout << this->getBody();
}

void			HttpRequest::checkRequest(void)
{
	if (DEBUG)
	{
		std::cout << "check request this->getMethod() = " << this->getMethod() << std::endl;
		std::cout << "check request this->getProtocol() = " << this->getProtocol() << std::endl;
	}

	if (this->getMethod() != "GET" && \
		this->getMethod() != "POST" && \
		this->getMethod() != "DELETE")
			this->setStatus(400);

	if (this->getProtocol().compare(0, 8,"HTTP/1.1") != 0)
		this->setStatus(400);

	if (DEBUG)
		std::cout << "check request this->getStatus() = " << this->getStatus() << std::endl;
}

std::string		getLineRequest(std::string &request)
{
	int len_line = request.find(REQUEST_EOL);
	std::string line = request.substr(0, len_line);
	request = request.substr(len_line + 2);
	return (line);
}

bool			isIpAddress(std::string host)
{
	if (std::count(host.begin(), host.end(), '.') != 3)
		return (0);
	// Pour différencier une IP d'un sous domaine ==> Par ex : www.sous.domaine.fr
	for (size_t i = 0; i < host.length(); i++)
	{
		if (std::isdigit(host[i]) == 0 && host[i] != '.')
			return (0);
	}
	for (size_t i = 0; i < 4; i++)
	{
		int	val = std::atoi(host.substr(0, host.find(".")).c_str());
		if (DEBUG == 1)
			std::cout << "val " << i << " = " << val << std::endl;
		if (val < 0 || val > 255)
			return (0);
		host = host.substr(host.find("."));
	}
	return (1);
}
