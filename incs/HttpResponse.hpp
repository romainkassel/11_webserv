#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

# include <iostream>
# include <unistd.h>
# include <map>
# include "HttpRequest.hpp"

class HttpResponse
{
	private:
		std::string		_protocol;
		std::string		_status_code;
		std::string		_status_text;

		std::map<std::string, std::string>	_headers;

		std::string		_fileToReturn;
		std::string		_body;

	public:
		HttpResponse(void);
		~HttpResponse(void);

		std::string		getFileToReturnBody(void);
		void			setFileToReturn(std::string);
		void			setProtocol(std::string);
		void			setStatus(int);
		void			setStatusCode(std::string);
		void			setStatusCode(int);
		void			setStatusText(std::string);
		void			addHeader(std::string, std::string);
		void			setBody(std::string);

		void			setIndexResponse(int);
		void			setBadRequest(void);

		std::string		getFileToReturn(void) const;
		std::string		getProtocol(void) const;
		std::string		getStatusCode(void) const;
		std::string		getStatusText(void) const;
		std::string		getHeader(std::string) const;
		std::string		getBody(void) const;

		std::string		build(void) const;

		void		printResponse(void) const;

		class HeaderNotFoundException : public std::exception
		{
		public:
			virtual const char* what() const throw()
			{
				return ("Error: Header not found.");
			}
		};
};


#endif

/*
	Response format :
	Start line (mandatory)				<PROTOCOL> <STATUS_CODE> <STATUS_TEXT>
	Response headers (optional)			<HEADER>: <VALUE(S)>
										...
	Representation headers (optional)	<HEADER>: <VALUE(S)>
		(only for GET response)		...
										<EMPTY_LINE>
	Response body (optional)			<DATA>
		(only for GET response)		...

	Status code : (https://developer.mozilla.org/en-US/docs/Web/HTTP/Status)
		- 100 - 199 Informational responses
		- 200 - 299 Successful responses
		- 300 - 399 Redirection messages
		- 400 - 499 Client error responses
		- 500 - 599 Server error responses

	Headers :
		- Response header : used only for responses to provide more informations and context.
		- Representation header :	used only for responses with body (GET method), describes how to
									interpret the data (format, encoding, length, etc...)

	Body :
		Data received with GET method, described in the representation headers
*/
