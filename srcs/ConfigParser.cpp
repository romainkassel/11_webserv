#include "../incs/ConfigParser.hpp"

ConfigParser::ConfigParser(std::string const &file)
{
	this->_server_nb = 0;
	this->_file_path = CONF_DIR + file;
	check_file(file);
	parse();
}

ConfigParser::~ConfigParser(void)
{
}

void	ConfigParser::check_file(std::string const &file) const
{
	if (file.substr(file.find_last_of(".") + 1) != "conf")
		throw ConfigParserException("Conf file wrong file extension");

	int	file_perm = access(this->_file_path.c_str(), R_OK | F_OK);
	if (file_perm < 0)
		throw ConfigParserException("Conf file path " + std::string(strerror(errno)));
}

std::string	ConfigParser::get_file_content(void)
{
	std::ifstream	ifs(this->_file_path.c_str());
	if (ifs.fail())
		throw ConfigParserException("Fail to open conf file: " + this->_file_path);

	std::stringstream	buffer;
	buffer << ifs.rdbuf();
	return buffer.str();
}

void	ConfigParser::remove_comments(std::string &content) const
{
	size_t	comment_pos = content.find('#');

	while (comment_pos != std::string::npos)
	{
		size_t	new_line = content.find('\n', comment_pos);
		content.erase(comment_pos, new_line - comment_pos);
		comment_pos = content.find('#');
	}
}

void	ConfigParser::parse(void)
{
	std::string	file_content = get_file_content();
	if (file_content.empty())
		throw ConfigParserException("Conf file is empty");

	remove_comments(file_content);
	split_servers(file_content);
	for (unsigned int i = 0; i < this->_server_nb; i++)
	{
		ConfigServer	server;
		configure_server(server, this->_servers_conf[i]);
		check_server(server);
		this->_servers.push_back(server);
	}
	check_duplicate_servers(this->_servers);
}

size_t	ConfigParser::find_block_start(size_t start_pos, std::string &content)
{
	size_t	pos;
	
	for (pos = start_pos; content[pos]; pos++)
	{
		if (std::isspace(content[pos])) continue;
		if (content[pos] == 's') break;
	}
	if (content[pos] == '\0') return start_pos;
	if (content.compare(pos, std::strlen("server"), "server") != 0)
		throw ConfigParserException("Wrong server directive in conf file");
	
	pos += std::strlen("server");
	while (content[pos] && std::isspace(content[pos]))
		pos++;
	if (content[pos] == '{') return pos;
	else throw ConfigParserException("Didn't find server block directive '{'");
}

size_t	ConfigParser::find_block_end(size_t start_pos, std::string &content)
{
	size_t	pos;
	size_t	sub_block = 0;

	for (pos = start_pos + 1; content[pos]; pos++)
	{
		if (content[pos] == '{') sub_block++;
		else if (content[pos] == '}' && sub_block > 0) sub_block--;
		else if (content[pos] == '}' && sub_block == 0) return pos;
	}
	return start_pos;
}

void	ConfigParser::split_servers(std::string &content)
{
	if (content.find("server {") == std::string::npos)
		throw ConfigParserException("Didn't find the server block directive start");

	size_t	pos = 0;
	size_t	start_pos = 0;
	size_t	end_pos = 1;

	while (start_pos != end_pos && start_pos < content.length())
	{
		start_pos = find_block_start(start_pos, content);
		end_pos = find_block_end(start_pos, content);
		if (start_pos == end_pos)
		{
			for (pos = start_pos; content[pos]; pos++)
			{
				if (std::isspace(content[pos]) == 0)
					throw ConfigParserException("Error in the server block");
			}
			break;
		}
		this->_servers_conf.push_back(content.substr(start_pos, end_pos - start_pos + 1));
		this->_server_nb++;
		start_pos = end_pos + 1;
	}
}

t_directive	ConfigParser::get_directive_code(std::string const &directive)
{
	if (directive == "{") return OPEN_BRACKET;
	if (directive == "}") return CLOSE_BRACKET;
	if (directive == "listen") return LISTEN;
	if (directive == "host") return HOST;
	if (directive == "server_name") return SERVER_NAME;
	if (directive == "root") return ROOT;
	if (directive == "index") return INDEX;
	if (directive == "client_max_body_size") return CLIENT_BODY_SIZE;
	if (directive == "error_page") return ERROR_PAGE;
	if (directive == "location") return LOCATION;
	return UNKNOWN;
}

std::vector<std::string>	ConfigParser::split_token(std::string &block, const char *delim)
{
	size_t	start;
	size_t	end = 0;
	std::vector<std::string>	tokens;

	while (true)
	{
		start = block.find_first_not_of(delim, end);
		if (start == std::string::npos) break;
		end = block.find_first_of(delim, start);
		tokens.push_back(block.substr(start, end - start));
	}
	return tokens;
}

std::list<ConfigServer>	ConfigParser::getServers(void)
{
	return (this->_servers);
}

void	ConfigParser::configure_server(ConfigServer &server, std::string &block)
{
	bool	in_location = false;
	std::vector<std::string>	tokens = split_token(block, " \n\t");
	std::vector<std::string>	error_page;

	for (size_t i = 0; i < tokens.size(); i++)
	{
		t_directive	directive_code = get_directive_code(tokens[i]);
		switch (directive_code)
		{
			case OPEN_BRACKET:
				continue;
			case CLOSE_BRACKET:
				break;
			case LISTEN:
			{
				if (!in_location)
				{
					if (server.get_port() != 0)
						throw ConfigParserException("Cannot have more than one 'listen' directive in the block");
					server.set_port(tokens[++i]);
				}
				break;
			}
			case HOST:
			{
				if (!in_location)
				{
					if (server.get_host() != "")
						throw ConfigParserException("Cannot have more than one 'host' directive in the block");
					server.set_host(tokens[++i]);
				}
				break;
			}
			case SERVER_NAME:
			{
				if (!in_location)
				{
					if (server.get_server_name() != "")
						throw ConfigParserException("Cannot have more than one 'server_name' directive in the block");
					server.set_server_name(tokens[++i]);
				}
				break;
			}
			case ROOT:
			{
				if (!in_location)
				{
					if (server.get_root() != "")
						throw ConfigParserException("Cannot have more than one 'root' directive in the block");
					server.set_root(tokens[++i]);
				}
				break;
			}
			case INDEX:
			{
				if (!in_location)
				{
					if (server.get_index() != "")
						throw ConfigParserException("Cannot have more than one 'index' directive in the block");
					server.set_index(tokens[++i]);
				}
				break;
			}
			case CLIENT_BODY_SIZE:
			{
				if (!in_location)
				{
					if (server.get_client_body_size() != 0)
						throw ConfigParserException("Cannot have more than one 'client_max_body_size' directive in the block");
					server.set_client_body_size(tokens[++i]);
				}
				break;
			}
			case ERROR_PAGE:
			{
				if (!in_location)
				{
					i++;
					while (i < tokens.size())
					{
						error_page.push_back(tokens[i]);
						if (tokens[i].find(';') != std::string::npos) break;
						i++;
					}
					if (i == tokens.size())
						throw ConfigParserException("Missing ';' at the end of '" + tokens[i] + "'");
				}
				break;
			}
			case LOCATION:
			{
				std::string	path;
				std::string	modifier;
				std::vector<std::string>	location_tokens;

				if (server.get_root() == "")
					throw ConfigParserException("Must set a 'root' before location blocks");
				++i;
				if (tokens[i] == "=" || tokens[i] == "^~")
				{
					modifier = tokens[i];
					i++;
				}
				else
					modifier = "none";
				if (tokens[i] == "{" || tokens[i] == "}")
					throw ConfigParserException("Must set a path before location blocks");
				path = tokens[i];
				i++;
				if (tokens[i] != "{")
					throw ConfigParserException("Expected an opening bracket '{' to start the location blocks.");
				i++;	
				while (i < tokens.size() && tokens[i] != "}")
				{
					location_tokens.push_back(tokens[i]);
					i++;
				}
				if (i == tokens.size())
					throw ConfigParserException("Expected an closing bracket '}' to end the location blocks.");
				server.set_location(location_tokens, path, modifier);
				in_location = true;
				break;
			}
			case UNKNOWN:
			{
				if (in_location == true)
					throw ConfigParserException("'location' block must be at the end of the 'server' block");
				throw ConfigParserException("Invalid tokens or directive" + tokens[i]);
			}
			default: throw ConfigParserException("Configure_server default state");
		}
	}
	server.set_error_pages(error_page);
	server.check_locations_duplicate(server.get_locations());
}

void	ConfigParser::check_server(ConfigServer &server)
{
	if (server.get_port() == 0)
		server.set_port(std::string(DEFAULT_SERVER_PORT));
	if (server.get_host() == "")
		server.set_host(std::string(DEFAULT_SERVER_HOST));
	if (server.get_client_body_size() == 0)
		server.set_client_body_size(std::string(DEFAULT_MAX_CLIENT_BODY_SIZE));
	if (server.get_index() == "")
		throw ConfigParserException("An index file must be specified");
	if (server.check_file(server.get_index(), server.get_root()) == false)
		throw ConfigParserException("Index file doesn't exist or couldn't be read");
}

void	ConfigParser::check_duplicate_servers(std::list<ConfigServer> &servers)
{
	std::set<std::string>	servers_name;
	std::list<ConfigServer>::const_iterator	cit = servers.begin();

	for (; cit != servers.end(); cit++)
	{
		if (!servers_name.insert(cit->get_server_name()).second)
			throw ConfigParserException("Same server name can't be defined more than once: " + cit->get_server_name());
	}

	std::set<uint16_t>	servers_port;
	for (cit = servers.begin(); cit != servers.end(); cit++)
	{
		if (!servers_port.insert(cit->get_port()).second)
		{
			std::stringstream ss;
			ss << cit->get_port();
			std::string	port = ss.str();
			throw ConfigParserException("Same server port can't be defined more than once: " + port);
		}
	}

	std::set<std::string>	servers_host;
	for (cit = servers.begin(); cit != servers.end(); cit++)
	{
		if (!servers_host.insert(cit->get_host()).second)
			throw ConfigParserException("Same server host can't be defined more than once: " + cit->get_host());
	}
}