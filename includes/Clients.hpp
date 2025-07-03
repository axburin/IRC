#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <ctime>

class Client {
	private :
		std::string nickname;
		std::string username;
		int client_fd;
		// time_t pause;
	
	public :
		Client(std::string nickname, std::string username, int fd);
		~Client(void);
};

#endif