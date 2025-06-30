#include "../incs/HttpResponse.hpp"
#include "ConfigParser.hpp"


HttpResponse::HttpResponse(void)
{
}

HttpResponse::~HttpResponse(void)
{
}

std::string	HttpResponse::getFileToReturnBody(void)
{
	std::ifstream	ifs(this->_fileToReturn.c_str());

	if (ifs.fail())
		throw ConfigParser::ConfigParserException("Fail to open html file: " + this->_fileToReturn);

	std::string 	line;
	std::string		fullLine;

    while (getline(ifs, line))
	{
		//std::cout << "line:" << line << std::endl;
		fullLine.append(line);
	}

	ifs.close();

	return (fullLine);
}

void		HttpResponse::setFileToReturn(std::string fileToReturn)
{
	this->_fileToReturn = fileToReturn;
}

void		HttpResponse::setProtocol(std::string protocol)
{
	this->_protocol = protocol;
}

void	HttpResponse::setStatus(int code)
{
	std::string	status_text;
	this->setStatusCode(code);
	switch (code)
	{
		case 100: status_text = ("Continue"); break;
		case 101: status_text = ("Switching Protocols"); break;
		case 103: status_text = ("Early Hints"); break;
		case 200: status_text = ("OK"); break;
		case 201: status_text = ("Created"); break;
		case 202: status_text = ("Accepted"); break;
		case 203: status_text = ("Non-Authoritative Information"); break;
		case 204: status_text = ("No Content"); break;
		case 205: status_text = ("Reset Content"); break;
		case 206: status_text = ("Partial Content"); break;
		case 300: status_text = ("Multiple Choices"); break;
		case 301: status_text = ("Moved Permanently"); break;
		case 302: status_text = ("Found"); break;
		case 303: status_text = ("See Other"); break;
		case 304: status_text = ("Not Modified"); break;
		case 307: status_text = ("Temporary Redirect"); break;
		case 308: status_text = ("Permanent Redirect"); break;
		case 400: status_text = ("Bad Request"); break;
		case 401: status_text = ("Unauthorized"); break;
		case 402: status_text = ("Payment Required"); break;
		case 403: status_text = ("Forbidden"); break;
		case 404: status_text = ("Not Found"); break;
		case 405: status_text = ("Method Not Allowed"); break;
		case 406: status_text = ("Not Acceptable"); break;
		case 407: status_text = ("Proxy Authentication Required"); break;
		case 408: status_text = ("Request Timeout"); break;
		case 409: status_text = ("Conflict"); break;
		case 410: status_text = ("Gone"); break;
		case 411: status_text = ("Length Required"); break;
		case 412: status_text = ("Precondition Failed"); break;
		case 413: status_text = ("Payload Too Large"); break;
		case 414: status_text = ("URI Too Long"); break;
		case 415: status_text = ("Unsupported Media Type"); break;
		case 416: status_text = ("Range Not Satisfiable"); break;
		case 417: status_text = ("Expectation Failed"); break;
		case 418: status_text = ("I'm a teapot"); break;
		case 421: status_text = ("Misdirected Request"); break;
		case 425: status_text = ("Too Early"); break;
		case 426: status_text = ("Upgrade Required"); break;
		case 428: status_text = ("Precondition Required"); break;
		case 429: status_text = ("Too Many Requests"); break;
		case 431: status_text = ("Request Header Fields Too Large"); break;
		case 451: status_text = ("Unavailable For Legal Reasons"); break;
		case 500: status_text = ("Internal Server Error"); break;
		case 501: status_text = ("Not Implemented"); break;
		case 502: status_text = ("Bad Gateway"); break;
		case 503: status_text = ("Service Unavailable"); break;
		case 504: status_text = ("Gateway Timeout"); break;
		case 505: status_text = ("HTTP Version Not Supported"); break;
		case 506: status_text = ("Variant Also Negotiates"); break;
		case 510: status_text = ("Not Extended"); break;
		case 511: status_text = ("Network Authentication Required"); break;
		default: status_text = ("Unknown");
	}
	this->setStatusText(status_text);
}

void		HttpResponse::setStatusCode(std::string status_code)
{
	this->_status_code = status_code;
}

void		HttpResponse::setStatusCode(int status_code)
{
	std::stringstream ss;
	ss << status_code;
	this->_status_code = ss.str();
}

void		HttpResponse::setStatusText(std::string status_text)
{
	this->_status_text = status_text;
}

void		HttpResponse::setIndexResponse(int status)
{
	this->setStatus(status);
	this->setProtocol("HTTP/1.1");
	this->addHeader("Server", "WebServ");
	this->addHeader("Connection", "Keep-Alive");
	this->addHeader("Keep-Alive", "timeout=5, max=997");
	this->addHeader("Content-Type", "text/html");
	std::string body = this->getFileToReturnBody();
	std::stringstream ss;
	ss << body.size();
	this->addHeader("Content-Length", ss.str());
	this->setBody(body);
}

void		HttpResponse::addHeader(std::string name, std::string value)
{
	this->_headers.insert(std::pair<std::string, std::string>(name, value));
}

void		HttpResponse::setBody(std::string body)
{
	this->_body = body;
}

std::string		HttpResponse::getFileToReturn(void) const
{
	return (this->_fileToReturn);
}

std::string		HttpResponse::getProtocol(void) const
{
	return (this->_protocol);
}

std::string		HttpResponse::getStatusCode(void) const
{
	return (this->_status_code);
}

std::string		HttpResponse::getStatusText(void) const
{
	return (this->_status_text);
}

std::string		HttpResponse::getHeader(std::string header) const
{
	mapItConst	it = this->_headers.find(header);
	if (it == this->_headers.end())
		throw HeaderNotFoundException();
	return ((*it).second);
}

std::string		HttpResponse::getBody(void) const
{
	return (this->_body);
}

void		HttpResponse::setBadRequest(void)
{
	this->setStatus(400);
	this->setProtocol("HTTP/1.1");
	this->addHeader("Server", "WebServ");
}

std::string		HttpResponse::build(void) const
{
	std::string	response;

	response.append(this->getProtocol());
	response.append(" ");
	response.append(this->getStatusCode());
	response.append(" ");
	response.append(this->getStatusText());
	response.append("\n");
	mapItConst	ite = this->_headers.end();
	for (mapItConst it = this->_headers.begin(); it != ite; it++)
	{
		response.append((*it).first);
		response.append(": ");
		response.append((*it).second);
		response.append("\n");
	}
	response.append("\n");
	if (!this->getBody().empty())
	{
		response.append(this->getBody());
		response.append("\n");
	}
	return (response);
}


void		HttpResponse::printResponse(void) const
{
	std::cout << this->getProtocol() << " " << this->getStatusCode() << " " << this->getStatusText() << std::endl;
	for (mapItConst it = this->_headers.begin(); it != this->_headers.end(); it++)
		std::cout << (*it).first << ": " << (*it).second << std::endl;
	std::cout << std::endl << this->getBody() << std::endl;
}
