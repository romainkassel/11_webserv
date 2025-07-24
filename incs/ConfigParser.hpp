#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

# include "WebServ.hpp"

typedef enum
{
	UNKNOWN = -1,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	LISTEN,
	HOST,
	SERVER_NAME,
	ROOT,
	INDEX,
	CLIENT_BODY_SIZE,
	ERROR_PAGE,
	LOCATION
}t_directive;

class	ConfigServer;

class ConfigParser
{
	public:
		ConfigParser(std::string const &file);
		~ConfigParser(void);

		void	check_file(std::string const &file) const;
		void	parse(void);
		void	remove_comments(std::string &content) const;
		void	split_servers(std::string &content);
		void	configure_server(ConfigServer &server, std::string &block);
		void	check_server(ConfigServer &server);
		void	check_duplicate_servers(std::list<ConfigServer> &servers);
		std::string	get_file_content(void);

		t_directive	get_directive_code(std::string const &directive);
		std::vector<std::string>	split_token(std::string &block, const char *delim);

		std::list<ConfigServer>	getServers(void);
		
		class	ConfigParserException : std::exception
		{
			public:
				ConfigParserException(std::string err) throw()
				{
					_err = "Parse Error: " + err;
				}
				virtual const char	*what(void) const throw()
				{
					return _err.c_str();
				}
				~ConfigParserException(void) throw() {}
			private:
				std::string	_err;
		};

	private:
		size_t	find_block_start(size_t start_pos, std::string &content);
		size_t	find_block_end(size_t start_pos, std::string &content);

	private:
		uint32_t	_server_nb;
		std::string	_file_path;
		std::vector<std::string>	_servers_conf;
		std::list<ConfigServer>	_servers;
};

#endif