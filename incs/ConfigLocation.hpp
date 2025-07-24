#ifndef CONFIG_LOCATION_HPP
#define CONFIG_LOCATION_HPP

# include "WebServ.hpp"

class	ConfigLocation
{
	public:
		ConfigLocation(void);
		ConfigLocation(ConfigLocation const & src);
		~ConfigLocation(void);

		ConfigLocation &operator=(ConfigLocation const & rhs);

		void	set_id(uint16_t id);
		void	set_is_default_location(bool is_default_location);
		void	set_is_cgi(bool is_cgi);
		void	set_redirection_code(uint16_t &redirection_code);
		void	set_modifier(std::string modifier);
		void	set_path(std::string path);
		void	set_root(std::string serv_root, std::string root);
		void	set_index(std::string index);
		void	set_auto_index(std::string auto_index);
		void	set_redirection_path(std::string path);
		void	set_upload_dir(std::string upload_dir);
		void	set_methods(std::vector<std::string> &methods);
		void	set_cgi_data(std::vector<std::string> &cgi_data);

		uint16_t	get_id(void) const;
		bool		get_is_default_location(void) const;
		bool		get_is_cgi(void) const;
		uint16_t	get_redirection_code(void) const;
		std::string	get_modifier(void) const;
		std::string	get_path(void) const;
		std::string	get_root(void) const;
		std::string	get_index(void) const;
		std::string	get_upload_dir(void) const;
		std::string	get_redirection_path(void) const;
		std::vector<std::string>	get_methods(void) const;

		void	display_info(void) const;
		void	check_location(std::string serv_root);
		bool	check_line_delim(std::string &value);
		bool	check_file(std::string &file, std::string const &root);

		class	ConfigLocationException : std::exception
		{
			public:
				ConfigLocationException(std::string err) throw()
				{
					_err = "ConfigLocation Error: " + err;
				}
				virtual const char	*what(void) const throw()
				{
					return _err.c_str();
				}
				~ConfigLocationException(void) throw() {}
			private:
				std::string	_err;
		};

	private:
		bool		_is_default_location;
		bool		_is_cgi;
		bool		_autoindex;
		uint16_t	_id;
		uint16_t	_redirection_code;
		std::string	_modifier;
		std::string	_path;
		std::string	_root;
		std::string	_index;
		std::string	_upload_dir;
		std::string	_redirection_path;
		std::vector<std::string>	_methods;
		std::map<std::string, std::string>	_cgi_data;
};

#endif