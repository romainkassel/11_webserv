#ifndef CONFIG_SERVER_HPP
#define CONFIG_SERVER_HPP

# include "WebServ.hpp"

typedef enum
{
	LOC_UNKNOWN = -1,
	LOC_ROOT,
	LOC_INDEX,
	AUTOINDEX,
	METHODS,
	REDIRECTION,
	CGI,
	UPLOAD_DIR
}t_directive_loc;

class ConfigLocation;

class	ConfigServer
{
	public:
		ConfigServer(void);
		ConfigServer(ConfigServer const & src);
		~ConfigServer(void);

		ConfigServer	&operator=(ConfigServer const & rhs);

		void	set_id(uint16_t id);
		void	set_port(std::string port);
		void	set_host(std::string host);
		void	set_server_name(std::string &server_name);
		void	set_root(std::string &root);
		void	set_index(std::string &index);
		void	set_target(std::string const &target);
		void	set_client_body_size(std::string client_body_size);
		void	set_error_pages(std::vector<std::string> &error_pages);
		void	setLocationIds(void);
		void	set_location(std::vector<std::string> &location_blocks, std::string const &path, std::string const &modifier);
		void	set_default_location(void);

		uint16_t	get_id(void) const;
		uint16_t	get_port(void) const;
		uint64_t	get_client_body_size(void) const;
		std::string	get_host(void) const;
		std::string	get_server_name(void) const;
		std::string	get_root(void) const;
		std::string	get_index(void) const;
		std::vector<ConfigLocation>	get_locations(void) const;
		std::map<uint16_t, std::string>	get_error_pages(void) const;
		std::string	get_file_to_return(void) const;
		ConfigLocation*	getWinnerLocation(void) const;
		
		void	init_default_error_pages(void);
		void	display_config(void) const;
		void	check_locations_duplicate(std::vector<ConfigLocation> locations);
		bool	check_file(std::string file, std::string const &root);
		bool	check_line_delim(std::string &value);
		t_directive_loc	get_loc_directive_code(std::string const &directive);
		uint32_t	string_to_int(std::string &str);
		std::string	get_status(uint32_t code);

		// Selection of location block

		void	getMatchingConfigLocation( void );
		void	getExactLocationMatches( void );
		void	getLongestLocationPrefixMatches( void );
		void	getFileToReturn( void );
		void	removeNonLongestMatches( long unsigned int const & longestMatch );
		void	displayLocationContainer( std::string const & locationContainerVersion ) const; // For tests

		class	ConfigServerException : std::exception
		{
			public:
				ConfigServerException(std::string err) throw()
				{
					_err = "ConfigServer Error: " + err;
				}
				virtual const char	*what(void) const throw()
				{
					return _err.c_str();
				}
				~ConfigServerException(void) throw() {}
			private:
				std::string	_err;
		};

	private:
		uint16_t	_id;
		uint16_t	_port;
		uint64_t	_client_body_size;
		std::string	_server_name;
		std::string _root;
		std::string	_index;
		std::string	_host;
		std::string	_target;
		std::string	_location_modifier;
		std::vector<ConfigLocation>	_locations;
		std::list<ConfigLocation>	_locationsTmp;
		bool						_is_location_found;
		ConfigLocation*				_winnerLocation;
		std::string	_fileToReturn;
		std::map<uint16_t, std::string>	_error_pages;
};

#endif