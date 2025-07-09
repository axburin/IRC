# include "Server.hpp"
# include "Channel.hpp"
# include "Clients.hpp"
# include "error.hpp"

// Constructeur par défaut
Client::Client():
	actual_channel(NULL), client_fd(-1), is_authenticated(false), has_password(false), 
	has_nick(false), has_user(false) {
}

Client::Client(std::string nickname, std::string username, int fd): 
	nickname(nickname), username(username), actual_channel(NULL), client_fd(fd), 
	is_authenticated(false), has_password(false), has_nick(false), has_user(false) {
}

Client::Client(int fd): 
	actual_channel(NULL), client_fd(fd), is_authenticated(false), has_password(false), 
	has_nick(false), has_user(false) {
}

Client::~Client(void){
}

bool Client::extractCommand(std::string& command) {
	// Chercher \r\n ou \n
	size_t pos = receive_buffer.find("\r\n");
	if (pos == std::string::npos) {
		pos = receive_buffer.find("\n");
		if (pos == std::string::npos) {
			return false;  // Pas de commande complète
		}
	}
	
	// Extraire la commande
	command = receive_buffer.substr(0, pos);
	
	// Retirer du buffer
	size_t erase_len = (receive_buffer[pos] == '\r') ? pos + 2 : pos + 1;
	receive_buffer.erase(0, erase_len);
	return true;
}

void Client::setNickname(const std::string& nick) {
	nickname = nick;
	has_nick = !nick.empty();
	// checkRegistration();
}

void Client::setUsername(const std::string& user) {
	username = user;
}

void Client::setRealname(const std::string& real) {
	realname = real;
	has_user = !username.empty() && !realname.empty();
	checkRegistration();
}

bool Client::isFullyRegistered() const {
	return has_password && has_nick && has_user && is_authenticated;
}

void Client::checkRegistration() {
	if (has_password && has_nick && has_user && !is_authenticated) {
		is_authenticated = true;
	}
}

std::string Client::getPrefix() const {
	return nickname + "!" + username + "@localhost";
}

void Client::setChannel(Channel* chan){
	actual_channel = chan;
}