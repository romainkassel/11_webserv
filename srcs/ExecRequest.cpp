# include "ExecRequest.hpp"
# include "RunServer.hpp"

void	execRequest(HttpRequest &request, HttpResponse &response)
{
	resetIndexPage(request, response);
	if (DEBUG)
	{
		std::cout << "execRequest" << std::endl;
		std::cout << "request.getMethod() = " << request.getMethod() << std::endl;
		std::cout << "request error status = " << request.getStatus() << std::endl;
	}
	if (request.getStatus() != 0)
	{
		response.setIndexResponse(request.getStatus());
		return ;
	}
	std::vector<std::string> allowed_methods = request.getWinnerServer()->getWinnerLocation()->get_methods();
	std::vector<std::string>::iterator it = std::find(allowed_methods.begin(), allowed_methods.end(), request.getMethod());
	if (it == allowed_methods.end())
	{
		if (DEBUG)
			std::cout << "method not allowed !" << std::endl;
		response.setIndexResponse(501);
		return ;
	}
	if (request.getMethod() == "GET")
		execGET(request, response);
	else if (request.getMethod() == "POST")
		execPOST(request, response);
	else if (request.getMethod() == "DELETE")
		execDELETE(request, response);
	if (DEBUG == 1)
		std::cout << "response : " << response.build() << std::endl;
}

void	execCGI_POST(HttpRequest &request, HttpResponse &response)
{
	if (DEBUG == 1)
		std::cout << "exec CGI POST" << std::endl;

	int		fd[2];
	int		pid;
	char	*env[1]; env[0] = NULL;
	int		status;

	std::string upload_dir = request.getWinnerServer()->getWinnerLocation()->get_upload_dir();

	std::string file = getTimestamp() + getFilenamePost(request);
	if (file.empty())
	{
		response.setIndexResponse(400);
		return;
	}

	std::string cgi_bin = request.getWinnerServer()->getWinnerLocation()->get_root();

	std::string target = getTargetPOST(request);

	std::string cgi = cgi_bin + target;

	if (DEBUG)
	{
		std::cout << "upload dir = " << upload_dir << std::endl;
		std::cout << "filename = " << file << std::endl;
		std::cout << "cgi_bin = " << cgi_bin << std::endl;
		std::cout << "target = " << target << std::endl;
		std::cout << "cgi = " << cgi << std::endl;
		std::cout << "access(cgi.c_str(), F_OK) = " << access(cgi.c_str(), F_OK) << std::endl;
	}

	if (access((cgi_bin + target).c_str(), F_OK) == -1)
	{
		response.setIndexResponse(501);
		return ;
	}
	if (DEBUG)
		std::cout << "cgi post ok" << std::endl;

	if (pipe(fd) < 0)
		return ;
	pid = fork();
	if (pid < 0)
		return ;
	if (pid == 0)
	{
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);
		char *cmd=const_cast<char *>(cgi.c_str());
		char *filename=const_cast<char *>(file.c_str());
		char *dirname=const_cast<char *>(upload_dir.c_str());
		char *args[]={cmd,filename,dirname,NULL};
		execve(cmd,args, env);
	}
	if (write(fd[1], request.getBody().c_str(), request.getBody().size()) <= 0)
	{
		response.setIndexResponse(500);
		return ;
	}
	close(fd[1]);
	close(fd[0]);
	waitpid(pid, &status, 0);

	if (WIFEXITED(status))
	status = WEXITSTATUS(status);
	if (DEBUG)
	std::cout << "status = " << status << std::endl;

	if (status == 0)
		response.setIndexResponse(201);
	else
		response.setIndexResponse(500);
	if (DEBUG == 1)
		std::cout << "body:" << response.getBody() << std::endl;
}

void	execCGI_GET(HttpRequest &request, HttpResponse &response)
{
	if (DEBUG)
		std::cout << "exec CGI GET" << std::endl;

	int		fd[2];
	int		pid;
	int		status;
	char	*env[1]; env[0] = NULL;

	std::string upload_dir = request.getWinnerServer()->getWinnerLocation()->get_upload_dir();

	std::string cgi_bin = request.getWinnerServer()->getWinnerLocation()->get_root();

	std::string target = getTargetGET(request);

	std::string cgi = cgi_bin + target;

	if (DEBUG)
	{
		std::cout << "upload dir = " << upload_dir << std::endl;
		std::cout << "cgi_bin = " << cgi_bin << std::endl;
		std::cout << "target = " << target << std::endl;
		std::cout << "cgi = " << cgi << std::endl;
		std::cout << "access(cgi.c_str(), F_OK) = " << access(cgi.c_str(), F_OK) << std::endl;
	}

	if (access(cgi.c_str(), F_OK) == -1)
	{
		response.setIndexResponse(501);
		return ;
	}

	if (DEBUG)
		std::cout << "cgi get ok" << std::endl;

	if (pipe(fd) < 0)
		return ;
	pid = fork();
	if (pid < 0)
		return ;
	if (pid == 0)
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		char *intrepreter=const_cast<char *>("/usr/bin/python3");
		char *cmd=const_cast<char *>(cgi.c_str());
		char *dirname=const_cast<char *>(upload_dir.c_str());
		char *args[]={intrepreter,cmd,dirname,NULL};
		execve(intrepreter,args, env);
	}
	std::string	body;
	char	buf[40];
	int		byte_read = 0;
	while (1)
	{
		int end_read = 0;
		byte_read = read(fd[0], buf, 40);
		if (byte_read == -1)
			break ;
		if (byte_read == 0)
			break ;
		for (size_t i = 0; i < 40; i++)
		{
			if (i < 40)
				body += (buf[i]);
			if (buf[i] == '\0')
			{
				end_read = 1;
				break;
			}
		}
		if (end_read)
			break;
	}
	close(fd[1]);
	close(fd[0]);
	waitpid(pid, &status, 0);

	if (WIFEXITED(status))
		status = WEXITSTATUS(status);
	if (DEBUG)
		std::cout << "status = " << status << std::endl;

	if (status == 0)
	{
		response.setProtocol("HTTP/1.1");
		response.addHeader("Server", "WebServ");
		response.addHeader("Content-Type", "text/html");
		response.setStatus(200);
		response.setBody(body);
		std::stringstream ss2;
		ss2 << body.size();
		response.addHeader("Content-Length", ss2.str());
	}
	else
		response.setIndexResponse(500);
	if (DEBUG)
		std::cout << "get debug body = " << body << std::endl;
}

void	execCGI_DELETE(HttpRequest &request, HttpResponse &response)
{
	if (DEBUG)
		std::cout << "exec CGI DELETE" << std::endl;
	int		fd[2];
	int		pid;
	int		status = 0;
	char	*env[1]; env[0] = NULL;

	std::string file = getFilenameDelete(request);

	std::string upload_dir = request.getWinnerServer()->getWinnerLocation()->get_upload_dir();

	std::string cgi_bin = request.getWinnerServer()->getWinnerLocation()->get_root();

	std::string target = getTargetDELETE(request);

	std::string cgi = cgi_bin + target;

	if (DEBUG)
	{
		std::cout << "filename = " << file << std::endl;
		std::cout << "upload dir = " << upload_dir << std::endl;
		std::cout << "cgi_bin = " << cgi_bin << std::endl;
		std::cout << "target = " << target << std::endl;
		std::cout << "cgi = " << cgi << std::endl;
		std::cout << "access(cgi.c_str(), F_OK) = " << access(cgi.c_str(), F_OK) << std::endl;
	}

	if (access(cgi.c_str(), F_OK) == -1)
	{
		response.setIndexResponse(501);
		return ;
	}

	if (DEBUG)
		std::cout << "cgi delete ok" << std::endl;

	if (pipe(fd) < 0)
		return ;
	pid = fork();
	if (pid < 0)
		return ;
	if (pid == 0)
	{
		close(fd[0]);
		close(fd[1]);
		char *cmd=const_cast<char *>(cgi.c_str());
		char *dirname=const_cast<char *>(upload_dir.c_str());
		char *filename=const_cast<char *>(file.c_str());
		char *args[]={cmd,dirname,filename,NULL};
		execve(cmd, args, env);
	}
	close(fd[0]);
	close(fd[1]);
	waitpid(pid, &status, 0);

	if (WIFEXITED(status))
		status = WEXITSTATUS(status);
	if (DEBUG)
		std::cout << "status = " << status << std::endl;
	if (status == 0)
		response.setIndexResponse(200);
	else
		response.setIndexResponse(400);
	if (DEBUG == 1)
		std::cout << "body:" << response.getBody() << std::endl;
}

void	execGET(HttpRequest &request, HttpResponse &response)
{
	if (DEBUG)
		std::cout << "exec GET" << std::endl;
	if (isCGIRequest(request.getTarget()))
		return (execCGI_GET(request, response));
	response.setIndexResponse(200);
	response.addHeader("Set-Cookie", create_cookie());
}

void	execPOST(HttpRequest &request, HttpResponse &response)
{
	if (DEBUG)
		std::cout << "exec POST" << std::endl;
	if (isCGIRequest(request.getTarget()))
		return execCGI_POST(request, response);
	response.setIndexResponse(200);
}

void	execDELETE(HttpRequest &request, HttpResponse &response)
{
	if (DEBUG)
		std::cout << "exec DELETE" << std::endl;
	if (isCGIRequest(request.getTarget()))
		return execCGI_DELETE(request, response);
	response.setIndexResponse(200);
}

bool	isCGIRequest(std::string const request)
{
	return (request.find("/cgi-bin/") != request.npos ? 1 : 0);
}

std::string	getFilenamePost(HttpRequest request)
{
	std::string header = request.getHeader("Content-Disposition");
	if (header.empty())
		return ("");
	std::string filename = header.substr(header.find("filename=\"") + 10);
	filename = filename.substr(0, filename.find("\""));
	for (size_t i = 0; i < filename.size(); i++)
		if (filename[i] == ' ')
			filename[i] = '_';
	return (filename);
}

std::string	getFilenameDelete(HttpRequest request)
{
	std::string target = request.getTarget();
	std::string filename = target.substr(target.find("Filename=") + 9);
	return (filename);
}

std::string	getTargetPOST(HttpRequest request)
{
	std::string target = request.getTarget();
	while (target.find("/cgi-bin") != target.npos)
		target = target.substr(8);
	target = target.substr(1, target.size() - 1);
	return (target);
}

std::string	getTargetGET(HttpRequest request)
{
	std::string target = request.getTarget();
	while (target.find("/cgi-bin") != target.npos)
		target = target.substr(8);
	target = target.substr(1, target.size() - 2);
	return (target);
}

std::string	getTargetDELETE(HttpRequest request)
{
	std::string target = request.getTarget();
	while (target.find("/cgi-bin") != target.npos)
		target = target.substr(8);
	target = target.substr(1, target.find('?') - 1);
	return (target);
}

std::string	getTimestamp(void)
{
	std::stringstream	ss;
	std::time_t	current_time = std::time(0);
	std::tm		*local_time = std::localtime(&current_time);

	ss	<< (local_time->tm_year + 1900)
		<< local_time->tm_mon
		<< local_time->tm_mday
		<< "_"
		<< local_time->tm_hour
		<< local_time->tm_min
		<< local_time->tm_sec;
	return (ss.str());
}

std::string	create_cookie(void)
{
	std::time_t	current_time = std::time(0);
	std::tm		*local_time = std::localtime(&current_time);

	std::tm	expiration_time;
	expiration_time.tm_year = local_time->tm_year;
	expiration_time.tm_mon = (local_time->tm_mon + 1) % 12;
	expiration_time.tm_mday = local_time->tm_mday;
	expiration_time.tm_hour = local_time->tm_hour;
	expiration_time.tm_min = local_time->tm_min;
	expiration_time.tm_isdst = local_time->tm_isdst;
	expiration_time.tm_sec = local_time->tm_sec;
    expiration_time.tm_wday = local_time->tm_wday;
    expiration_time.tm_yday = local_time->tm_yday;

	char	expiration[EXPIRATION_TIME_BUFFER_SIZE];
	std::string	cookie_info;

	std::strftime(expiration, EXPIRATION_TIME_BUFFER_SIZE, "%a, %d-%b-%Y %H:%M:%S", &expiration_time);

	cookie_info = "cookie_name=cookie_value; HttpOnly; Expires=" + std::string(expiration) + " GMT";
	return cookie_info;
}

void	resetIndexPage(HttpRequest &request, HttpResponse &response)
{
	if (response.getFileToReturn().compare((*request.getWinnerServer()).get_error_pages()[413]) == 0)
	{
		if (DEBUG == 1)
		std::cout << "413 error page found" << std::endl;
		return ;
	}

	if (DEBUG == 1)
		std::cout << "response.getFileToReturn() AVANT: " << response.getFileToReturn() << std::endl;

	if (DEBUG == 1)
	{
		std::cout << "(*request.getWinnerServer()).get_root(): " << (*request.getWinnerServer()).get_root() << std::endl;
		std::cout << "(*request.getWinnerServer()).get_index(): " << (*request.getWinnerServer()).get_index() << std::endl;
		std::cout << "(*request.getWinnerServer()).get_root().length(): " << (*request.getWinnerServer()).get_root().length() << std::endl;
		std::cout << "(*request.getWinnerServer()).get_root()[(*request.getWinnerServer()).get_root().length(): " << (*request.getWinnerServer()).get_root()[(*request.getWinnerServer()).get_root().length() - 1] << std::endl;
		std::cout << "(*request.getWinnerServer()).get_index()[0]: " << (*request.getWinnerServer()).get_index()[0] << std::endl;
	}

	if ((*request.getWinnerServer()).get_root()[(*request.getWinnerServer()).get_root().length() - 1] != '/'
		&& (*request.getWinnerServer()).get_index()[0] != '/')
	{
		std::string newIndex = "/" + (*request.getWinnerServer()).get_index() + ";";
		(*request.getWinnerServer()).set_index(newIndex);
	}

	std::string	fileToReturn = RunServer::removeMultipleSlashs((*request.getWinnerServer()).get_root() + (*request.getWinnerServer()).get_index());
	response.setFileToReturn(fileToReturn);

	if (DEBUG == 1)
		std::cout << "response.getFileToReturn() APRES: " << response.getFileToReturn() << std::endl;
}
