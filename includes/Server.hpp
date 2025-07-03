#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>
#include <set>
#include <map>

class Client;
class Channel;

class Server {
	private :
		// std::map<std::string, Client> clients;
		// std::map<std::string, Channel> channels;
		// int epoll_fd;
		std::string password;

	public : 
		Server(/*int port,*/ std::string password);
		~Server(void);
};

#endif