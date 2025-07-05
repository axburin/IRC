#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>
#include <set>
#include <map>
#include <exception>

class Client;
class Channel;

class Server {
	private :
		std::map<std::string, Client> clients;
		std::map<std::string, Channel> channels;
		int epoll_fd;
		int server_fd;
		std::vector<int> fds;
		std::string password;

	public : 
		Server(int port, std::string password);
		~Server(void);

		void listenServer(void);

		class ServerErrorException : public std::exception {
			std::string msg;

			public :
				ServerErrorException(const std::string& msg);
				virtual ~ServerErrorException() throw();
				virtual const char* what(void) const throw();
		};
};

#endif