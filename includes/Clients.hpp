#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <ctime>
#include <set>

class Channel;

class Client {
	private :
		std::set<std::string> invite;
		std::string nickname;
		std::string username;
		std::string realname;
		Channel *actual_channel;
		int client_fd;
		bool is_authenticated;
		bool has_password;
		bool has_nick;
		bool has_user;
		std::string receive_buffer; // Buffer simple temporaire
	
	public :
		Client(); // Constructeur par défaut
		Client(std::string nickname, std::string username, int fd);
		Client(int fd); // Pour clients non encore authentifiés
		~Client(void);
		
		// Getters
		Channel* getChannel(void)const {return actual_channel; }
		const std::string& getNickname() const { return nickname; }
		const std::string& getUsername() const { return username; }
		const std::string& getRealname() const { return realname; }
		int getFd() const { return client_fd; }
		bool isAuthenticated() const { return is_authenticated; }
		
		// Buffer simple
		void addToBuffer(const std::string& data) { receive_buffer += data; }
		bool extractCommand(std::string& command);
		
		// Setters
		void setChannel(Channel* chan);
		void setNickname(const std::string& nick);
		void setUsername(const std::string& user);
		void setRealname(const std::string& real);
		void setPassword(bool correct) { has_password = correct; }		
		// Authentification
		bool isFullyRegistered() const;
		void checkRegistration();
		
		// Utilitaires
		std::string getPrefix() const;

};

#endif