#ifndef WEBSERV_HPP
#define WEBSERV_HPP

# include <stdint.h>
# include <linux/limits.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/stat.h>
# include <dirent.h>
# include <unistd.h>
# include <iostream>
# include <limits>
# include <cstdlib>
# include <cstring>
# include <fstream>
# include <sstream>
# include <vector>
# include <map>
# include <list>
# include <set>

# include "ConfigParser.hpp"
# include "ConfigServer.hpp"
# include "ConfigLocation.hpp"

# define UNUSED(x) (void)x

# define WEBSERV_USAGE "Usage: ./webserv <config file> (optional)"
# define DEFAULT_CONF_FILE "default.conf"
# define CONF_DIR "./conf/"

# define DEFAULT_SERVER_PORT "8080;"
# define DEFAULT_SERVER_HOST "localhost;"

# define DEFAULT_MAX_CLIENT_BODY_SIZE "1000000;"
# define DEFAULT_LOCATION_MODIFIER "="
# define DEFAULT_LOCATION_PATH "/"
# define DEFAULT_LOCATION_ROOT "default/;"
# define DEFAULT_LOCATION_INDEX "index.html;"
# define DEFAULT_UPLOAD_DIR "default_upload_dir"

# define EXPIRATION_TIME_BUFFER_SIZE 50

template<typename T>
void	display_vector(T vector)
{
	for (typename T::const_iterator cit = vector.begin(); cit != vector.end(); cit++)
		std::cout << *cit << '\n';
	std::cout << std::endl;
}

template<typename T>
void	display_map(T map)
{
	for (typename T::const_iterator cit = map.begin(); cit != map.end(); cit++)
	{
		std::cout << "Key: " << cit->first << '\n';
		std::cout << "Value: " << cit->second << '\n';
	}
	std::cout << std::endl;
}

#endif