#include "ConfigServer.hpp"
#include "RunServer.hpp"

ConfigServer::ConfigServer(void)
{
	this->_id = 0;
	this->_port = 0;
	this->_client_body_size = 0;
	this->_server_name = "";
	this->_root = "";
	this->_index = "";
	this->_host	= "";
	this->_location_modifier = "";
	this->_is_location_found = false;
	this->_error_pages.clear();
	this->_locations.clear();
	init_default_error_pages();
}

ConfigServer::ConfigServer(ConfigServer const & src)
{
	if (DEBUG == 1)
	{
		std::cout << "Copy constructor from ConfigServer called" << std::endl;
		std::cout << std::endl;
	}

	*this = src;
}

ConfigServer::~ConfigServer(void)
{
}

ConfigServer	&ConfigServer::operator=(ConfigServer const & rhs)
{
	if (DEBUG == 1)
	{
		std::cout << "Affectation operator from ConfigServer called" << std::endl;
		std::cout << std::endl;
	}

	if (this != &rhs)
	{
		this->_id = rhs._id;
		this->_host = rhs._host;
		this->_port = rhs._port;
		this->_client_body_size = rhs._client_body_size;
		this->_server_name = rhs._server_name;
		this->_target = rhs._target;
		this->_root = rhs._root;
		this->_index = rhs._index;
		this->_is_location_found = rhs._is_location_found;
		this->_location_modifier = rhs._location_modifier;
		if (DEBUG == 1)
		{
			std::cout << "this->_error_pages.size(): " << this->_error_pages.size() << std::endl;
			std::cout << "this->_error_pages[404]: " << this->_error_pages[404] << std::endl;
		}
		this->_error_pages = rhs._error_pages;
		this->_locations = rhs._locations;
		this->_winnerLocation = rhs._winnerLocation;
	}

	return (*this);
}

void	ConfigServer::set_id(uint16_t id)
{
	this->_id = id;
}

void	ConfigServer::set_port(std::string port)
{
	if (check_line_delim(port) == false)
		throw ConfigServerException("Missing ';' at the end of '" + port + "'");
	uint16_t	port_number = string_to_int(port);
	if (port_number < 1 || port_number > std::numeric_limits<uint16_t>::max())
		throw ConfigServerException("Port value is out_of_range");
	this->_port = port_number;
}

void	ConfigServer::set_host(std::string host)
{
	if (check_line_delim(host) == false)
		throw ConfigServerException("Missing ';' at the end of '" + host + "'");
	this->_host = host;
}

void	ConfigServer::set_server_name(std::string &server_name)
{
	if (check_line_delim(server_name) == false)
		throw ConfigServerException("Missing ';' at the end of '" + server_name + "'");
	this->_server_name = server_name;
}

void	ConfigServer::set_root(std::string &root)
{
	if (check_line_delim(root) == false)
		throw ConfigServerException("Missing ';' at the end of '" + root + "'");
	struct stat	file_stat;
	if (stat(root.c_str(), &file_stat) == 0 && file_stat.st_mode & S_IFDIR)
	{
		this->_root = root;
		return;
	}

	char	buffer[PATH_MAX];
	if (getcwd(buffer, PATH_MAX) == false)
		throw ConfigServerException("Set_root getcwd failed");

	std::string	path = buffer + root;
	if (stat(path.c_str(), &file_stat) == 0 && file_stat.st_mode & S_IFDIR)
		this->_root = path;
	else
		throw ConfigServerException("Cannot find the path: " + path);
}

void	ConfigServer::set_index(std::string &index)
{
	if (check_line_delim(index) == false)
		throw ConfigServerException("Missing ';' at the end of '" + index + "'");
	this->_index = index;
}

void	ConfigServer::set_target(std::string const &target)
{
	this->_target = target;
}

void	ConfigServer::set_client_body_size(std::string client_body_size)
{
	if (check_line_delim(client_body_size) == false)
		throw ConfigServerException("Missing ';' at the end of '" + client_body_size + "'");
	uint64_t	size = string_to_int(client_body_size);
	if (size < 1)
		throw ConfigServerException("Client body size value is out_of_range");
	this->_client_body_size = size;
}

void	ConfigServer::set_error_pages(std::vector<std::string> &error_pages)
{
	if (error_pages.empty() == true) return;

	uint16_t	error_code;
	std::string	file_path;
	if (error_pages.size() % 2 != 0)
		throw ConfigServerException("Wrong error_pages pattern in conf file must be: <error_code> <file_path>");

	for (size_t i = 0; i < error_pages.size(); i++)
	{
		if (i % 2 == 0)
		{
			if (error_pages[i].length() == 3)
			{
				error_code = string_to_int(error_pages[i]);
				if (get_status(error_code) == "Unknown")
					throw ConfigServerException("Invalid http error code: " + error_pages[i]);
			}
			else
				throw ConfigServerException("Invalid http error code: " + error_pages[i]);
		}
		else
		{
			file_path = error_pages[i];
			if (i == error_pages.size() - 1)
			{
				if (check_line_delim(file_path) == false)
					throw ConfigServerException("Missing ';' at the end of '" + file_path + "'");
			}
			if (check_file(file_path, this->_root) == false)
				throw ConfigServerException("Invalid error page path: " + file_path);

			std::map<uint16_t, std::string>::const_iterator	cit = this->_error_pages.find(error_code);
			if (cit == this->_error_pages.end())
				this->_error_pages.insert(std::make_pair(error_code, file_path));
			else
				this->_error_pages[error_code] = file_path;
		}
	}
}

void	ConfigServer::set_default_location(void)
{
	ConfigLocation	default_location;
	std::vector<std::string>	default_method;

	default_method.push_back("GET");
	default_method.push_back("POST");
	default_method.push_back("DELETE");

	default_location.set_is_default_location(true);
	default_location.set_modifier(DEFAULT_LOCATION_MODIFIER);
	default_location.set_path(DEFAULT_LOCATION_PATH);
	default_location.set_root(this->_root, this->_root + ";");
	//default_location.set_root(this->_root, DEFAULT_LOCATION_ROOT);
	default_location.set_index(DEFAULT_LOCATION_INDEX);
	default_location.set_methods(default_method);
	default_location.set_upload_dir(std::string(DEFAULT_UPLOAD_DIR).append(";"));

	this->_locations.push_back(default_location);
}

t_directive_loc	ConfigServer::get_loc_directive_code(std::string const &directive)
{
	if (directive == "root") return LOC_ROOT;
	if (directive == "index") return LOC_INDEX;
	if (directive == "autoindex") return AUTOINDEX;
	if (directive == "allow_methods") return METHODS;
	if (directive == "return") return REDIRECTION;
	if (directive == "cgi") return CGI;
	if (directive == "upload_dir") return UPLOAD_DIR;
	return LOC_UNKNOWN;
}

void	ConfigServer::set_location(std::vector<std::string> &location_blocks, std::string const &path, std::string const &modifier)
{
	bool	autoindex_setted = false;
	ConfigLocation	location;
	std::vector<std::string>	methods;
	std::vector<std::string>	cgi_data;

	location.set_modifier(modifier);
	location.set_path(path);
	if (path == "/cgi-bin") location.set_is_cgi(true);
	for (size_t i = 0; i < location_blocks.size(); i++)
	{
		t_directive_loc	loc_directive = get_loc_directive_code(location_blocks[i]);
		switch (loc_directive)
		{
			case LOC_ROOT:
			{
				if (location.get_root() != "")
					throw ConfigServerException("Cannot have more than one 'root' directive in the block");
				location.set_root(this->_root, location_blocks[++i]);
				break;
			}
			case LOC_INDEX:
			{
				if (location.get_index() != "")
					throw ConfigServerException("Cannot have more than one 'index' directive in the block");
				location.set_index(location_blocks[++i]);
				break;
			}
			case AUTOINDEX:
			{
				if (autoindex_setted == true)
					throw ConfigServerException("Cannot have more than one 'autoindex' directive in the block");
				if (location.get_is_cgi() == true)
					throw ConfigServerException("'autoindex' directive is incompatible with 'cgi-bin' location");
				location.set_auto_index(location_blocks[++i]);
				autoindex_setted = true;
				break;
			}
			case METHODS:
			{
				if (location.get_methods().empty() == false)
					throw ConfigServerException("Cannot have more than one 'allow_methods' directive in the block");
				++i;
				while (i < location_blocks.size())
				{
					if (location_blocks[i].find(';') != std::string::npos)
					{
						if (check_line_delim(location_blocks[i]) == false)
							throw ConfigServerException("Missing ';' at the end of '" + location_blocks[i] + "'");
						methods.push_back(location_blocks[i]);
						break;
					}
					else
						methods.push_back(location_blocks[i]);
					if (i + 1 == location_blocks.size())
						throw ConfigServerException("Missing ';' at the end of '" + location_blocks[i] + "'");
					i++;
				}
				location.set_methods(methods);
				break;
			}
			case REDIRECTION:
			{
				if (location.get_is_cgi() == true)
					throw ConfigServerException("'return' directive is incompatible with '/cgi-bin' location");
				if (location.get_redirection_code() != 0)
					throw ConfigServerException("Cannot have more than one 'return' directive in the block");
				++i;
				uint16_t	redirection_code = string_to_int(location_blocks[i]);
				if (redirection_code >= 300 && redirection_code <= 308)
					location.set_redirection_code(redirection_code);
				else
					throw ConfigServerException("Unknown redirection code after 'return' directive : " + location_blocks[i]);
				location.set_redirection_path(location_blocks[++i]);
				break;
			}
			case CGI:
			{
				i++;
				while (i < location_blocks.size())
				{
					cgi_data.push_back(location_blocks[i]);
					if (location_blocks[i].find(';') != std::string::npos) break;
					i++;
				}
				if (i == location_blocks.size())
					throw ConfigServerException("Missing ';' at the end of '" + location_blocks[i] + "'");
				break;
			}
			case UPLOAD_DIR:
			{
				if (location.get_upload_dir() != "")
					throw ConfigServerException("Cannot have more than one 'upload_dir' directive in the block");
				location.set_upload_dir(location_blocks[++i]);
				break;
			}
			case LOC_UNKNOWN:
			{
				throw ConfigServerException("Unknown or incomplete directive in 'location' context : " + location_blocks[i]);
				break;
			}
		}
	}
	if (location.get_is_cgi() == true) location.set_cgi_data(cgi_data);
	location.check_location(this->_root);
	this->_locations.push_back(location);
}

void	ConfigServer::setLocationIds(void)
{
	RunServer::displayFunctionTitle("setLocationIds");

	std::vector<ConfigLocation>::iterator	it;

	int i = 0;

	for (it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		(*it).set_id(i++);
	}
}

uint16_t	ConfigServer::get_id(void) const
{
	return this->_id;
}

uint16_t	ConfigServer::get_port(void) const
{
	return this->_port;
}

uint64_t	ConfigServer::get_client_body_size(void) const
{
	return this->_client_body_size;
}

std::string	ConfigServer::get_host(void) const
{
	return this->_host;
}

std::string	ConfigServer::get_server_name(void) const
{
	return this->_server_name;
}

std::string	ConfigServer::get_root(void) const
{
	return this->_root;
}

std::string	ConfigServer::get_index(void) const
{
	return this->_index;
}

std::vector<ConfigLocation>	ConfigServer::get_locations(void) const
{
	return this->_locations;
}

std::map<uint16_t, std::string>	ConfigServer::get_error_pages(void) const
{
	return this->_error_pages;
}

std::string	ConfigServer::get_file_to_return(void) const
{
	return this->_fileToReturn;
}

void	ConfigServer::init_default_error_pages(void)
{
	this->_error_pages.clear();
	this->_error_pages[403] = "./error_pages/403.html";
	this->_error_pages[404] = "./error_pages/404.html";
	this->_error_pages[413] = "./error_pages/413.html";
}

void	ConfigServer::display_config(void) const
{
	std::cout << "===Config Server Info===" << std::endl;

	std::cout << std::endl;

	std::cout << BOLD << WHITE;
	std::cout << "ConfigServer - ID " << this->_id << END << std::endl;

	std::cout << std::endl;

	std::cout << "Host: " << this->_host << std::endl;
	std::cout << "Port: " << this->_port << std::endl;
	std::cout << "Server name: " << this->_server_name << std::endl;
	std::cout << "Client body size: " << this->_client_body_size << std::endl;
	std::cout << "Root: " << this->_root << std::endl;
	std::cout << "Index: " << this->_index << std::endl;

	if (this->_is_location_found == true)
		std::cout << "Is_location_found: true" << std::endl;
	else
		std::cout << "Is_location_found: false" << std::endl;

	std::cout << std::endl;

	if (this->_locations.empty() == false)
		this->_locations[0].display_info();
}

void	ConfigServer::check_locations_duplicate(std::vector<ConfigLocation> locations)
{
	if (locations.empty() == true)
	{
		set_default_location();
		return;
	}

	std::set<std::string>	tmp;
	for (size_t i = 0; i < locations.size(); i++)
	{
		if (!tmp.insert(locations[i].get_path()).second)
			throw ConfigServerException("Same location path can't be defined for the same path: " + locations[i].get_path());
	}
}

bool	ConfigServer::check_file(std::string file, std::string const &root)
{
	struct stat	file_stat;
	if (stat(file.c_str(), &file_stat) == 0 && file_stat.st_mode & S_IFREG
		&& access(file.c_str(), R_OK) == 0)
			return true;

	std::string	full_path = root + "/" + file;
	if (stat(full_path.c_str(), &file_stat) == 0 && file_stat.st_mode & S_IFREG
		&& access(full_path.c_str(), R_OK) == 0)
			return true;
	return false;
}

bool	ConfigServer::check_line_delim(std::string &value)
{
	if (value.rfind(';') == std::string::npos || value[0] == ';') return false;
	value.erase(value.length() - 1);
	return true;
}

uint32_t	ConfigServer::string_to_int(std::string &str)
{
	if (str.length() > 10) return 0;

	for (size_t i = 0; i < str.length(); i++)
		if (!std::isdigit(str[i]))
			return 0;

	uint32_t	number = std::atoi(str.c_str());
	return number;
}

std::string	ConfigServer::get_status(uint32_t code)
{

	switch (code)
	{
		case 100: return ("Continue");
		case 101: return ("Switching Protocols");
		case 103: return ("Early Hints");
		case 200: return ("OK");
		case 201: return ("Created");
		case 202: return ("Accepted");
		case 203: return ("Non-Authoritative Information");
		case 204: return ("No Content");
		case 205: return ("Reset Content");
		case 206: return ("Partial Content");
		case 300: return ("Multiple Choices");
		case 301: return ("Moved Permanently");
		case 302: return ("Found");
		case 303: return ("See Other");
		case 304: return ("Not Modified");
		case 307: return ("Temporary Redirect");
		case 308: return ("Permanent Redirect");
		case 400: return ("Bad Request");
		case 401: return ("Unauthorized");
		case 402: return ("Payment Required");
		case 403: return ("Forbidden");
		case 404: return ("Not Found");
		case 405: return ("Method Not Allowed");
		case 406: return ("Not Acceptable");
		case 407: return ("Proxy Authentication Required");
		case 408: return ("Request Timeout");
		case 409: return ("Conflict");
		case 410: return ("Gone");
		case 411: return ("Length Required");
		case 412: return ("Precondition Failed");
		case 413: return ("Payload Too Large");
		case 414: return ("URI Too Long");
		case 415: return ("Unsupported Media Type");
		case 416: return ("Range Not Satisfiable");
		case 417: return ("Expectation Failed");
		case 418: return ("I'm a teapot");
		case 421: return ("Misdirected Request");
		case 425: return ("Too Early");
		case 426: return ("Upgrade Required");
		case 428: return ("Precondition Required");
		case 429: return ("Too Many Requests");
		case 431: return ("Request Header Fields Too Large");
		case 451: return ("Unavailable For Legal Reasons");
		case 500: return ("Internal Server Error");
		case 501: return ("Not Implemented");
		case 502: return ("Bad Gateway");
		case 503: return ("Service Unavailable");
		case 504: return ("Gateway Timeout");
		case 505: return ("HTTP Version Not Supported");
		case 506: return ("Variant Also Negotiates");
		case 510: return ("Not Extended");
		case 511: return ("Network Authentication Required");
		default: return ("Unknown");
	}
}

// Selection of location block

void	ConfigServer::getMatchingConfigLocation( void )
{
	RunServer::displayFunctionTitle("getMatchingConfigLocation");

	this->_locationsTmp.clear();
	this->_winnerLocation = NULL;
	this->_is_location_found = false;
	this->_fileToReturn = "";

	if ((*this->_locations.begin()).get_is_default_location() == true && this->_target.compare("/") == 0)
	{
		this->_winnerLocation = &(*this->_locations.begin());

		if (DEBUG == 1)
			std::cout << "this->_locations.size(): " << this->_locations.size() << std::endl;
	}
	else
	{
		std::list<ConfigLocation>	tmp(this->_locations.begin(), this->_locations.end());
		this->_locationsTmp = tmp;

		this->getExactLocationMatches();

		if (this->_is_location_found == false)
		{
			if (DEBUG == 1)
				std::cout << "CONDITION 1" << std::endl;

			this->getLongestLocationPrefixMatches();
		}

		if (DEBUG == 1)
			std::cout << "this->_target: " << this->_target << std::endl;

		if (this->_is_location_found == false)
		{
			this->set_default_location();
			this->_locationsTmp.clear();
			this->_locationsTmp.push_back(*this->_locations.rbegin());

			if (this->_target.compare("/") != 0)
			{
				if (DEBUG == 1)
					std::cout << "CONDITION 2" << std::endl;

				this->_winnerLocation = &(*this->_locationsTmp.begin());
				this->_fileToReturn = this->_error_pages[404];

				return ;
			}
		}

		this->_winnerLocation = &(*this->_locationsTmp.begin());
	}

	this->getFileToReturn();

	if (DEBUG == 1)
	{
		std::cout << "this->_locationsTmp.size(): " << this->_locationsTmp.size() << std::endl;
		std::cout << std::endl;

		if (this->_locationsTmp.size() == 1 || this->_winnerLocation != NULL)
		{
			std::cout << BOLD << WINNER;
			std::cout << "Winning and matching ConfigLocation is: " << std::endl;
			std::cout << END;
			std::cout << std::endl;
			this->_winnerLocation->display_info();
		}
		else
		{
			std::cerr << NORMAL << ERROR;
			std::cerr << "ERROR: still no location selected (at least 2 in competition)"<< std::endl;
			std::cerr << std::endl;
			std::cerr << END;
		}
	}
}

void	ConfigServer::getExactLocationMatches( void )
{
	RunServer::displayFunctionTitle("getExactLocationMatches");

	std::list<ConfigLocation>::iterator	it;
	ConfigLocation						ConfigLocationTmp;

	if (DEBUG == 1)
		std::cout << "this->_locationsTmp.size(): " << this->_locationsTmp.size() << std::endl;

	for (it = this->_locationsTmp.begin(); it != this->_locationsTmp.end(); ++it)
	{
		if (DEBUG == 1)
		{
			std::cout << std::endl;
			(*it).display_info();
			std::cout << "this->_target: " << this->_target << std::endl;
		}

		if ((*it).get_path().compare(this->_target) == 0 && (*it).get_modifier().compare("=") == 0)
		{
			ConfigLocationTmp = *it;
			this->_locationsTmp.clear();
			this->_locationsTmp.push_back(ConfigLocationTmp);
			this->_is_location_found = true;

			break;
		}
	}

	//this->displayLocationContainer("cpy");
}

void	ConfigServer::getLongestLocationPrefixMatches( void )
{
	RunServer::displayFunctionTitle("getLongestLocationPrefixMatches");

	std::list<ConfigLocation>			locationsTmpCpy;
	std::list<ConfigLocation>::iterator	it;
	std::string							tmp;
	int									end;
	long unsigned int					longestMatch;

	locationsTmpCpy = this->_locationsTmp;
	longestMatch = 0;

	if (DEBUG == 1)
	{
		std::cout << "this->_locationsTmp.size(): " << this->_locationsTmp.size() << std::endl;
		std::cout << std::endl;
	}

	for (it = this->_locationsTmp.begin(); it != this->_locationsTmp.end(); ++it)
	{
		if (DEBUG == 1)
			std::cout << "this->_target: " << this->_target << std::endl;

		tmp = "";
		end = (*it).get_path().length();

		if ((*it).get_modifier().compare("=") != 0)
		{
			tmp = this->_target.substr(0, end);

			if (DEBUG == 1)
			{
				std::cout << "tmp                  : " << tmp << std::endl;
				std::cout << "(*it).get_path(): " << (*it).get_path() << std::endl;
			}
		}

		if (DEBUG == 1)
			std::cout << std::endl;

		if (tmp.compare((*it).get_path()) != 0)
			this->_locationsTmp.erase(it--);
		else if (tmp.length() > longestMatch)
			longestMatch = tmp.length();
	}

	if (DEBUG == 1)
	{
		std::cout << "this->_locationsTmp.size(): " << this->_locationsTmp.size() << std::endl;
		std::cout << "longestMatch: " << longestMatch << std::endl;
		std::cout << std::endl;
	}

	//this->displayLocationContainer("cpy");

	if (this->_locationsTmp.size() > 1)
		this->removeNonLongestMatches(longestMatch);

	if (this->_locationsTmp.size() == 0)
		this->_locationsTmp = locationsTmpCpy;
	else
		this->_is_location_found = true;

	//this->displayLocationContainer("cpy");
}

void	ConfigServer::removeNonLongestMatches( long unsigned int const & longestMatch )
{
	RunServer::displayFunctionTitle("removeNonLongestMatches");

	if (DEBUG == 1)
	{
		std::cout << "this->_locationsTmp.size(): " << this->_locationsTmp.size() << std::endl;
		std::cout << std::endl;
	}

	std::list<ConfigLocation>::iterator	it;

	for (it = this->_locationsTmp.begin(); it != this->_locationsTmp.end(); ++it)
		if ((*it).get_path().length() < longestMatch)
			this->_locationsTmp.erase(it--);

	//this->displayLocationContainer("cpy");

	if (DEBUG == 1)
	{
		std::cout << "this->_locationsTmp.size(): " << this->_locationsTmp.size() << std::endl;
		std::cout << std::endl;
	}
}

void	ConfigServer::getFileToReturn( void )
{
	RunServer::displayFunctionTitle("getFileToReturn");

	struct stat sb;

	if (this->_winnerLocation->get_redirection_path().empty() != true)
	{
		this->_fileToReturn = RunServer::removeMultipleSlashs(this->_winnerLocation->get_root() + this->_winnerLocation->get_redirection_path());
		return ;
	}

	if (DEBUG == 1)
	{
		std::cout << "this->_winnerLocation->get_root(): " << this->_winnerLocation->get_root() << std::endl;
		std::cout << "this->_target: " << this->_target << std::endl;
		std::cout << "this->_winnerLocation->get_index(): " << this->_winnerLocation->get_index() << std::endl;
		std::cout << "this->_index: " << this->_index << std::endl;
		std::cout << std::endl;
	}

	this->_fileToReturn = this->_winnerLocation->get_root() + this->_target;

	std::size_t	found = this->_target.find(".html");

	if (found == std::string::npos)
	{
		std::string	delim;

		if (this->_winnerLocation->get_index().empty() == true)
		{
			delim = ";";
			this->_winnerLocation->set_index(this->_index + delim);
		}

		delim = "/";
		this->_fileToReturn += delim;
		this->_fileToReturn += this->_winnerLocation->get_index();
	}

	if (DEBUG == 1)
	{
		std::cout << "this->_fileToReturn BEFORE: " << this->_fileToReturn << std::endl;
		std::cout << std::endl;
	}


	found = this->_fileToReturn.find("//");
	while (found != std::string::npos)
	{
		this->_fileToReturn.replace(found,2,"/");
		found = this->_fileToReturn.find("//");
	}

	if (DEBUG == 1)
	{
		std::cout << "this->_fileToReturn AFTER: " << this->_fileToReturn << std::endl;
		std::cout << std::endl;
	}

	if (stat(this->_fileToReturn.c_str(), &sb) != 0)
	{
		if (DEBUG == 1)
		{
			std::cout << "File to return does not exist" << std::endl;
			std::cout << "this->_error_pages.size(): " << this->_error_pages.size() << std::endl;
			std::cout << "this->_error_pages[404]: " << this->_error_pages[404] << std::endl;
		}

		this->_fileToReturn = this->_error_pages[404];
	}
}

ConfigLocation*	ConfigServer::getWinnerLocation(void) const
{
	return (this->_winnerLocation);
}

// For tests

void	ConfigServer::displayLocationContainer( std::string const & locationContainerVersion ) const
{
	std::list<ConfigLocation>					locationContainer;
	std::list<ConfigLocation>::const_iterator	it;

	if (locationContainerVersion.compare("cpy") == 0)
	{
		locationContainer = this->_locationsTmp;
	}
	else
	{
		std::list<ConfigLocation>	tmp(this->_locations.begin(), this->_locations.end());

		locationContainer = tmp;
	}

	if (DEBUG == 1)
		std::cout << "locationContainer.size(): " << locationContainer.size() << std::endl;

	std::cout << std::endl;

	for (it = locationContainer.begin(); it != locationContainer.end(); ++it)
		(*it).display_info();
}