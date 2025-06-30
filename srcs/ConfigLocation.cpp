#include "ConfigLocation.hpp"
#include "RunServer.hpp"

ConfigLocation::ConfigLocation(void)
{
	this->_id = 0;
	this->_is_default_location = false;
	this->_is_cgi = false;
	this->_autoindex = false;
	this->_redirection_code = 0;
	this->_modifier = "";
	this->_path = "";
	this->_root = "";
	this->_index = "";
	this->_upload_dir = "";
	this->_redirection_path = "";
	this->_methods.clear();
	this->_cgi_data.clear();
}

ConfigLocation::ConfigLocation(ConfigLocation const & src)
{
	if (DEBUG == 1)
	{
		std::cout << "Copy constructor from ConfigLocation called" << std::endl;
		std::cout << std::endl;
	}

	*this = src;
}

ConfigLocation::~ConfigLocation(void)
{
}

ConfigLocation	&ConfigLocation::operator=(ConfigLocation const & rhs)
{
	if (DEBUG == 1)
	{
		std::cout << "Affectation operator from ConfigLocation called" << std::endl;
		std::cout << std::endl;
	}

	if (this != &rhs)
	{
		this->_autoindex = rhs._autoindex;
		this->_is_default_location = rhs._is_default_location;
		this->_is_cgi = rhs._is_cgi;
		this->_id = rhs._id;
		this->_modifier = rhs._modifier;
		this->_path = rhs._path;
		this->_autoindex = rhs._autoindex;
		this->_methods = rhs._methods;
		this->_index = rhs._index;
		this->_cgi_data = rhs._cgi_data;
		this->_redirection_path = rhs._redirection_path;
		this->_redirection_code = rhs._redirection_code;
		this->_root = rhs._root;
		this->_upload_dir = rhs._upload_dir;
	}

	return (*this);
}

void	ConfigLocation::set_id(uint16_t id)
{
	this->_id = id;
}

void	ConfigLocation::set_is_default_location(bool is_default_location)
{
	this->_is_default_location = is_default_location;
}

void	ConfigLocation::set_is_cgi(bool is_cgi)
{
	this->_is_cgi = is_cgi;
}

void	ConfigLocation::set_redirection_code(uint16_t &redirection_code)
{
	this->_redirection_code = redirection_code;
}

void	ConfigLocation::set_modifier(std::string modifier)
{
	this->_modifier = modifier;
}

void	ConfigLocation::set_path(std::string path)
{
	this->_path = path;
}

void	ConfigLocation::set_root(std::string serv_root, std::string root)
{
	if (check_line_delim(root) == false)
		throw ConfigLocationException("Missing ';' at the end of '" + root + "'");
	struct stat	file_stat;
	// if (stat(root.c_str(), &file_stat) == 0 && file_stat.st_mode & S_IFDIR)
	// {
	// 	this->_root = root;
	// 	return;
	// }

	std::string	full_root = serv_root + root;
	if (stat(full_root.c_str(), &file_stat) == 0 && file_stat.st_mode & S_IFDIR)
		this->_root = full_root;
	else
		throw ConfigLocationException("Cannot find the path: " + full_root);
}

void	ConfigLocation::set_index(std::string index)
{
	if (check_line_delim(index) == false)
		throw ConfigLocationException("Missing ';' at the end of '" + index + "'");
	this->_index = index;
}

void	ConfigLocation::set_auto_index(std::string auto_index)
{
	if (auto_index == "on;") this->_autoindex = true;
	else if (auto_index != "off;")
		throw ConfigLocationException("Autoindex must set only to 'on' or 'off'");
}

void	ConfigLocation::set_redirection_path(std::string path)
{
	if (check_line_delim(path) == false)
		throw ConfigLocationException("Missing ';' at the end of '" + path + "'");
	if (check_file(path, this->_root) == false)
		throw ConfigLocationException("Cannot find the path: '" + this->_root + path + "'");
	this->_redirection_path = path;
}

void	ConfigLocation::set_upload_dir(std::string upload_dir)
{
	if (check_line_delim(upload_dir) == false)
		throw ConfigLocationException("Missing ';' at the end of '" + upload_dir + "'");
	this->_upload_dir = upload_dir;
}

void	ConfigLocation::set_methods(std::vector<std::string> &methods)
{
	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] != "GET" && methods[i] != "POST" && methods[i] != "DELETE")
			throw ConfigLocationException("Invalid methods in 'allow_methods' line: " + methods[i]);
		else
			this->_methods.push_back(methods[i]);
	}

	std::set<std::string>	tmp;
	for (size_t i = 0; i < this->_methods.size(); i++)
	{
		if (!tmp.insert(this->_methods[i]).second)
			throw ConfigLocationException("Cannot allow same methods more than once");
	}
}

void	ConfigLocation::set_cgi_data(std::vector<std::string> &cgi_data)
{
	if (cgi_data.empty() == true)
		throw ConfigLocationException("'cgi' directive with appropriate information needed in 'cgi-bin' location");

	std::string	cgi_ext;
	std::string	cgi_path;
	if (cgi_data.size() % 2 != 0)
		throw ConfigLocationException("Wrong cgi pattern in conf file must be: <extension> <extension_path>");
	if (cgi_data.size() > 4)
		throw ConfigLocationException("Wrong cgi pattern in conf file handling only 2 type of cgi");
	if (cgi_data.size() == 4 && cgi_data[0] == cgi_data[2])
		throw ConfigLocationException("Duplicate file extension after 'cgi' directive in 'cgi-bin' location");
	for (size_t i = 0; i < cgi_data.size(); i++)
	{
		if (i % 2 == 0)
		{
			cgi_ext = cgi_data[i];
			if (cgi_ext != ".cpp" && cgi_ext != ".py")
				throw ConfigLocationException("Invalid cgi extention only '.cpp' and '.py' are allowed");
		}
		else
		{
			cgi_path = cgi_data[i];
			if (i == cgi_data.size() - 1)
			{
				if (check_line_delim(cgi_path) == false)
					throw ConfigLocationException("Missing ';' at the end of '" + cgi_path + "'");
			}
			if (check_file(cgi_path, "/") == false)
				throw ConfigLocationException("Invalid cgi path: " + cgi_path);
			this->_cgi_data.insert(std::make_pair(cgi_ext, cgi_path));
		}
	}
}

uint16_t	ConfigLocation::get_id(void) const
{
	return this->_id;
}

bool	ConfigLocation::get_is_default_location(void) const
{
	return this->_is_default_location;
}

bool	ConfigLocation::get_is_cgi(void) const
{
	return this->_is_cgi;
}

uint16_t	ConfigLocation::get_redirection_code(void) const
{
	return this->_redirection_code;
}

std::string	ConfigLocation::get_modifier(void) const
{
	return this->_modifier;
}

std::string	ConfigLocation::get_path(void) const
{
	return this->_path;
}

std::string	ConfigLocation::get_root(void) const
{
	return this->_root;
}

std::string	ConfigLocation::get_index(void) const
{
	return this->_index;
}

std::string	ConfigLocation::get_redirection_path(void) const
{
	return this->_redirection_path;
}

std::string	ConfigLocation::get_upload_dir(void) const
{
	return this->_upload_dir;
}

std::vector<std::string>	ConfigLocation::get_methods(void) const
{
	return this->_methods;
}

void	ConfigLocation::display_info(void) const
{
	std::cout << "===Location Info===" << std::endl;

	std::cout << std::endl;

	std::cout << BOLD << WHITE;
	std::cout << "ConfigLocation - ID " << this->_id << END << std::endl;

	std::cout << std::endl;

	if (this->_is_default_location == true)
		std::cout << "Is_default_location: true" << std::endl;
	else
		std::cout << "Is_default_location: false" << std::endl;

	if (this->_is_cgi == true)
		std::cout << "Is_cgi: true" << std::endl;
	else
		std::cout << "Is_cgi: false" << std::endl;

	if (this->_autoindex == false)
		std::cout << "Autoindex: false" << std::endl;
	else
		std::cout << "Autoindex: true" << std::endl;

	std::cout << "Modifier: " << this->_modifier << std::endl;
	std::cout << "Path: " << this->_path << std::endl;
	std::cout << "Redirection_path: " << this->_redirection_path << std::endl;
	std::cout << "Root: " << this->_root << std::endl;
	std::cout << "Index: " << this->_index << std::endl;
	std::cout << "Redirection code : "  << this->_redirection_code << std::endl;
	std::cout << "Redirection path : "  << this->_redirection_path << std::endl;
	std::cout << "Upload dir : "  << this->_upload_dir << std::endl;
	std::cout << "Allow Methods: " << std::endl;
	std::cout << "Upload_dir dir: " << this->_upload_dir << std::endl;
	display_vector(this->_methods);
}

void	ConfigLocation::check_location(std::string serv_root)
{
	if (this->_root == "") this->_root = serv_root;
	if (this->_autoindex == true && this->_index != "")
		throw ConfigLocationException("Location cannot have a 'index' if autoindex is on");
	if (this->_is_cgi == true && this->_index == "")
		throw ConfigLocationException("Missing 'index' directive in 'cgi-bin' directive");
	if (this->_path[0] != '/')
		throw ConfigLocationException("Invalid 'root' directive in 'location' directive: " + this->_path);
	if (this->_methods.empty() == true)
		throw ConfigLocationException("Empty methods in 'allow_methods' directive in 'location' directive");
	if (this->_autoindex == false && this->_root == serv_root && this->_index != "" && check_file(this->_index, this->_root + this->_path) == false)
		throw ConfigLocationException("Invalid 'index' file path in 'location' directive: " + this->_index);
	if (this->_autoindex == false && this->_root != serv_root && this->_index != "" && check_file(this->_index, this->_root) == false)
		throw ConfigLocationException("Invalid 'index' file path in 'location' directive: " + std::string(this->_root + this->_index));
	if (this->_upload_dir == "")
	{
		struct stat	tmp;
		std::string	default_dir = this->_root + DEFAULT_UPLOAD_DIR;
		if (stat(default_dir.c_str(), &tmp) != 0 || !S_ISDIR(tmp.st_mode) || access(default_dir.c_str(), R_OK | W_OK) != 0)
			throw ConfigLocationException("Default upload dir didn't exist: " + default_dir);
		set_upload_dir(default_dir.append(";"));
	}
	else if (this->_upload_dir != "")
	{
		struct stat	tmp;
		std::string	dir_path = this->_root + this->_upload_dir;
		if (stat(dir_path.c_str(), &tmp) != 0 || !S_ISDIR(tmp.st_mode) || access(dir_path.c_str(), R_OK | W_OK) != 0)
			throw ConfigLocationException("Invalid upload directory path: " + this->_root + this->_upload_dir);
		this->_upload_dir = dir_path;
	}
}

bool	ConfigLocation::check_line_delim(std::string &value)
{
	if (value.rfind(';') != value.length() - 1 || value[0] == ';') return false;
	value.erase(value.length() - 1);
	return true;
}

bool	ConfigLocation::check_file(std::string &file, std::string const &root)
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