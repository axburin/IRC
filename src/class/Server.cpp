#include "Server.hpp"
#include "Channel.hpp"
#include "Clients.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include "error.hpp"
#include <sys/epoll.h>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>

// Fonction utilitaire pour convertir int en string (C++98)
std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Parser simple temporaire
std::vector<std::string> simpleParse(const std::string& command) {
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::string token;
    
    // Parser mot par mot jusqu'à rencontrer ":"
    while (iss >> token) {
        if (!token.empty() && token[0] == ':') {
            // Tout le reste fait partie du message
            tokens.push_back(token);
            std::string remaining;
            std::getline(iss, remaining);
            if (!remaining.empty()) {
                tokens.back() += remaining;
            }
            break;
        } else {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

Server::Server(int port, std::string password ): password(password){
	sockaddr_in server_addr;
	epoll_event event;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		throw ServerErrorException("Socket error");
	
	// Réutilisation d'adresse
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		close(server_fd);
		throw ServerErrorException("Bind error");
	}
	if (listen(server_fd, SOMAXCONN) < 0){
		close(server_fd);
		throw ServerErrorException("Listen error");
	}
	setNonBlocking(server_fd);
	epoll_fd = epoll_create1(0);
	if (epoll_fd < 0){
		close(server_fd);
		throw ServerErrorException("Epoll_create1 error");
	}
	event.events = EPOLLIN;
	event.data.fd = server_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
}

Server::~Server(void){
	close(server_fd);
	close(epoll_fd);
	for (std::vector<int>::iterator it = fds.begin(); it != fds.end(); ++it){
		close(*it);
	}
}

void Server::listenServer(void){
	std::vector<epoll_event> events(10);

	while (true) {
		int n = epoll_wait(epoll_fd, events.data(), 10, -1);
		for (int i = 0; i < n; ++i){
			if (events[i].data.fd == server_fd) {
				acceptNewClient();
			} else {
				handleClientData(events[i].data.fd);
			}
		}
	}
}

void Server::acceptNewClient() {
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
	
	if (client_fd >= 0) {
		setNonBlocking(client_fd);
		epoll_event client_event;
		client_event.events = EPOLLIN | EPOLLET;
		client_event.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);
		fds.push_back(client_fd);
		
		// Créer un client temporaire
		std::string temp_key = "temp_" + intToString(client_fd);
		clients.insert(std::make_pair(temp_key, Client(client_fd)));
		
		std::cout << "Nouveau client connecte : fd = " << client_fd << std::endl;
	}
}

void Server::handleClientData(int client_fd) {
	char buffer[1024];
	int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
	
	if (bytes_read <= 0) {
		std::cout << "Client deconnecte: fd = " << client_fd << std::endl;
		disconnectClient(client_fd);
		return;
	}
	
	buffer[bytes_read] = '\0';
	std::string data(buffer);
	
	// Trouver le client
	Client* client = findClientByFd(client_fd);
	if (!client) {
		std::cout << "Client non trouvé pour fd " << client_fd << std::endl;
		return;
	}
	
	// Ajouter les données au buffer du client
	client->addToBuffer(data);
	
	// Extraire et traiter tous les messages complets
	std::string command;
	while (client->extractCommand(command)) {
		std::cout << "Commande reçue: " << command << std::endl;
		processCommand(client, command);
	}
}

void Server::processCommand(Client* client, const std::string& command) {
	std::vector<std::string> tokens = simpleParse(command);
	
	if (tokens.empty()) {
		return;
	}
	
	std::string cmd = toUpper(tokens[0]);
	
	if (cmd == "PASS") {
		handlePass(client, tokens);
	} else if (cmd == "NICK") {
		handleNick(client, tokens);
	} else if (cmd == "USER") {
		handleUser(client, tokens);
	} else if (cmd == "QUIT") {
		handleQuit(client, tokens);
	} else if (!client->isFullyRegistered()) {
		sendError(client, "451", ":You have not registered");
	} else {
		// Commandes nécessitant une authentification complète
		if (cmd == "JOIN") {
			handleJoin(client, tokens);
		} else if (cmd == "PRIVMSG") {
			handlePrivmsg(client, tokens);
		} else if (cmd == "PART") {
			handlePart(client, tokens);
		} else {
			sendError(client, "421", cmd + " :Unknown command");
		}
	}
}

void Server::handlePass(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() < 2) {
		sendError(client, "461", "PASS :Not enough parameters");
		return;
	}
	
	if (tokens[1] == password) {
		client->setPassword(true);
		std::cout << "Client " << client->getFd() << " mot de passe correct" << std::endl;
	} else {
		sendError(client, "464", ":Password incorrect");
	}
}

void Server::handleNick(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() < 2) {
		sendError(client, "431", ":No nickname given");
		return;
	}
	
	std::string nick = tokens[1];
	
	// Vérifier si le nick est déjà utilisé
	if (isNickInUse(nick)) {
		sendError(client, "433", nick + " :Nickname is already in use");
		return;
	}
	
	client->setNickname(nick);
	std::cout << "Client " << client->getFd() << " nickname: " << nick << std::endl;
}

void Server::handleUser(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() < 5) {
		sendError(client, "461", "USER :Not enough parameters");
		return;
	}
	
	client->setUsername(tokens[1]);
	client->setRealname(tokens[4]);
	
	std::cout << "Client " << client->getFd() << " user: " << tokens[1] << std::endl;
	
	if (client->isFullyRegistered()) {
		sendWelcomeMessages(client);
	}
}

void Server::handleQuit(Client* client, const std::vector<std::string>& tokens) {
	(void)tokens; // Éviter warning unused parameter
	std::cout << "Client " << client->getNickname() << " quitte" << std::endl;
	disconnectClient(client->getFd());
}

void Server::handleJoin(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() < 2) {
		sendError(client, "461", "JOIN :Not enough parameters");
		return;
	}
	
	std::string channel_name = tokens[1];
	
	// Vérifier que c'est un nom de canal valide
	if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&')) {
		sendError(client, "403", channel_name + " :No such channel");
		return;
	}
	
	std::cout << "Client " << client->getNickname() << " rejoint " << channel_name << std::endl;
	
	// Pour l'instant, juste confirmer le JOIN (sans vraie gestion des canaux)
	std::string join_msg = ":" + client->getPrefix() + " JOIN " + channel_name + "\r\n";
	send(client->getFd(), join_msg.c_str(), join_msg.length(), 0);
	
	// Envoyer un topic vide
	sendReply(client, "331", client->getNickname() + " " + channel_name + " :No topic is set");
	
	// Envoyer la liste des noms (juste le client pour l'instant)
	sendReply(client, "353", client->getNickname() + " = " + channel_name + " :@" + client->getNickname());
	sendReply(client, "366", client->getNickname() + " " + channel_name + " :End of NAMES list");
}

void Server::handlePrivmsg(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() < 2) {
		sendError(client, "411", ":No recipient given");
		return;
	}
	
	// Reconstituer le message (tout après le target)
	std::string target = tokens[1];
	std::string message = "";
	
	// Trouver le début du message (après ":")
	for (size_t i = 2; i < tokens.size(); i++) {
		if (i == 2 && !tokens[i].empty() && tokens[i][0] == ':') {
			// Enlever le ":" du début
			message = tokens[i].substr(1);
		} else {
			if (!message.empty()) message += " ";
			message += tokens[i];
		}
	}
	
	if (message.empty()) {
		sendError(client, "412", ":No text to send");
		return;
	}
	
	std::cout << "PRIVMSG de " << client->getNickname() 
			  << " vers " << target << ": " << message << std::endl;
	
	if (target[0] == '#' || target[0] == '&') {
		// Message vers un canal - pour l'instant, juste echo vers l'expéditeur
		std::cout << "Message canal: " << target << std::endl;
		
		// Echo vers l'expéditeur pour confirmer (temporaire)
		std::string echo = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
		send(client->getFd(), echo.c_str(), echo.length(), 0);
		
	} else {
		// Message privé vers un utilisateur
		Client* target_client = findClientByNick(target);
		if (target_client) {
			std::string privmsg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
			send(target_client->getFd(), privmsg.c_str(), privmsg.length(), 0);
		} else {
			sendError(client, "401", target + " :No such nick/channel");
		}
	}
}

void Server::handlePart(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() < 2) {
		sendError(client, "461", "PART :Not enough parameters");
		return;
	}
	
	std::string channel_name = tokens[1];
	
	// Reconstituer la raison (optionnelle)
	std::string reason = client->getNickname(); // Raison par défaut
	
	if (tokens.size() > 2) {
		reason = "";
		for (size_t i = 2; i < tokens.size(); i++) {
			if (i == 2 && !tokens[i].empty() && tokens[i][0] == ':') {
				reason = tokens[i].substr(1);
			} else {
				if (!reason.empty()) reason += " ";
				reason += tokens[i];
			}
		}
	}
	
	std::cout << "Client " << client->getNickname() << " quitte " << channel_name 
			  << " (" << reason << ")" << std::endl;
	
	// Confirmer le PART
	std::string part_msg = ":" + client->getPrefix() + " PART " + channel_name + " :" + reason + "\r\n";
	send(client->getFd(), part_msg.c_str(), part_msg.length(), 0);
}

// Ajouter cette fonction utilitaire pour trouver par nickname
Client* Server::findClientByNick(const std::string& nickname) {
	for (std::map<std::string, Client>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
		if (it->second.getNickname() == nickname) {
			return &(it->second);
		}
	}
	return NULL;
}

// Utilitaires
Client* Server::findClientByFd(int fd) {
	for (std::map<std::string, Client>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
		if (it->second.getFd() == fd) {
			return &(it->second);
		}
	}
	return NULL;
}

bool Server::isNickInUse(const std::string& nickname) {
	for (std::map<std::string, Client>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
		if (it->second.getNickname() == nickname) {
			return true;
		}
	}
	return false;
}

void Server::sendError(Client* client, const std::string& code, const std::string& message) {
	std::string response = ":ircserv " + code + " " + message + "\r\n";
	send(client->getFd(), response.c_str(), response.length(), 0);
}

void Server::sendWelcomeMessages(Client* client) {
	std::string nick = client->getNickname();
	
	sendReply(client, "001", nick + " :Welcome to IRC " + client->getPrefix());
	sendReply(client, "002", nick + " :Your host is ircserv");
	sendReply(client, "003", nick + " :This server was created today");
	sendReply(client, "004", nick + " ircserv 1.0 o itko");
}

void Server::sendReply(Client* client, const std::string& code, const std::string& message) {
	std::string response = ":ircserv " + code + " " + message + "\r\n";
	send(client->getFd(), response.c_str(), response.length(), 0);
}

void Server::disconnectClient(int client_fd) {
	// Retirer de epoll
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	
	// Fermer le socket
	close(client_fd);
	
	// Retirer de la liste des fds
	for (std::vector<int>::iterator it = fds.begin(); it != fds.end(); ++it) {
		if (*it == client_fd) {
			fds.erase(it);
			break;
		}
	}
	
	// Retirer de la map des clients
	for (std::map<std::string, Client>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
		if (it->second.getFd() == client_fd) {
			clients.erase(it);
			break;
		}
	}
}

// Exception
Server::ServerErrorException::ServerErrorException(const std::string& msg): msg(msg){}
Server::ServerErrorException::~ServerErrorException() throw(){}
const char *Server::ServerErrorException::what() const throw(){
	return msg.c_str();
}