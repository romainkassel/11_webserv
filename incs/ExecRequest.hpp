#ifndef EXECREQUEST_HPP
# define EXECREQUEST_HPP

# include <iostream>
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include <sstream>
# include <sys/wait.h>
# include <ctime>

void	execRequest(HttpRequest &, HttpResponse &);
void	execGET(HttpRequest &, HttpResponse &);
void	execCGI(HttpRequest &, HttpResponse &);
void	execPOST(HttpRequest &, HttpResponse &);
void	execDELETE(HttpRequest &, HttpResponse &);

bool	isCGIRequest(std::string const);

std::string	getFilenamePost(HttpRequest);
std::string	getFilenameDelete(HttpRequest);
std::string	getTargetPOST(HttpRequest);
std::string	getTargetGET(HttpRequest);
std::string	getTargetDELETE(HttpRequest);
std::string	getTimestamp(void);

std::string	create_cookie(void);

void	resetIndexPage(HttpRequest &request, HttpResponse &response);

#endif
