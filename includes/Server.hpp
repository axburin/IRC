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
		
		// Nouvelles méthodes
		void acceptNewClient();
		void handleClientData(int client_fd);
		bool processCommand(Client* client, const std::string& command);
		void disconnectClient(int client_fd);
		
		// Handlers de commandes (version simple)
		void handlePass(Client* client, const std::vector<std::string>& tokens);
		void handleNick(Client* client, const std::vector<std::string>& tokens);
		void handleUser(Client* client, const std::vector<std::string>& tokens);
		void handleQuit(Client* client, const std::vector<std::string>& tokens);
		void handleJoin(Client* client, const std::vector<std::string>& tokens);
		void handlePrivmsg(Client* client, const std::vector<std::string>& tokens);
		void handlePart(Client* client, const std::vector<std::string>& tokens);
		
		// Utilitaires
		Client* findClientByFd(int fd);
		bool isNickInUse(const std::string& nickname);
		void sendError(Client* client, const std::string& code, const std::string& message);
		void sendReply(Client* client, const std::string& code, const std::string& message);
		void sendWelcomeMessages(Client* client);
		Client* findClientByNick(const std::string& nickname);

		class ServerErrorException : public std::exception {
			std::string msg;

			public :
				ServerErrorException(const std::string& msg);
				virtual ~ServerErrorException() throw();
				virtual const char* what(void) const throw();
		};
};

#endif