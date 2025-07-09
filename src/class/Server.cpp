#include "Server.hpp"
#include "Channel.hpp"
#include "Clients.hpp"
#include "error.hpp"
#include "irc_utils.hpp"


Server::Server(int port, std::string password ): password(password) {
	sockaddr_in server_addr;
	epoll_event event;
	running = true;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		throw ServerErrorException("Socket error");
	
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
	for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		delete it->second;
	}
		for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		delete it->second;
	}
}
void Server::stop() {
	running = false;
}

void Server::listenServer(void){
	std::vector<epoll_event> events(10);

	while (running) {
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
		std::string temp_key = "temp_" + intToString(client_fd);
		clients.insert(std::make_pair(temp_key, new Client(client_fd)));
		
		std::cout << "New client : fd = " << client_fd << std::endl;
	}
}

void Server::handleClientData(int client_fd) {
	char buffer[1024];
	int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
	
	if (bytes_read <= 0) {
		std::cout << "Client disconnected : fd = " << client_fd << std::endl;
		disconnectClient(client_fd);
		return;
	}
	buffer[bytes_read] = '\0';
	std::string data(buffer);
	Client* client = findClientByFd(client_fd);
	if (!client) {
		std::cout << "Client not found" << client_fd << ". Data ignored." << std::endl;
		return;
	}
	client->addToBuffer(data);
	std::string command;
	while (client->extractCommand(command)) {
		std::cout << "Command receive: " << command << std::endl;
		
		if (!processCommand(client, command)) {
			return;
		}
		client = findClientByFd(client_fd);
		if (!client) {
			std::cout << "Client deleted after process, end of program." << std::endl;
			return;
		}
	}
}

bool Server::processCommand(Client* client, const std::string& command) {
	std::vector<std::string> tokens = simpleParse(command);
	
	if (tokens.empty()) {
		return true;
	}
	
	std::string cmd = tokens[0];
	
	if (cmd == "PASS") {
		handlePass(client, tokens);
	} else if (cmd == "INVITE") {
		handleInvite(client, tokens);
	} else if (cmd == "KICK") {
		handleKick(client, tokens);
	} else if (cmd == "NICK") {
		Handle_Nick(client, tokens);
	} else if (cmd == "USER") {
		handleUser(client, tokens);
	} else if (cmd == "QUIT") {
		handleQuit(client, tokens);
		return false;
	} else if (!client->isFullyRegistered()) {
		sendError(client, "451", ":You have not registered");
	} else {
		if (cmd == "JOIN") {
			handleJoin(client, tokens);
		} else if (cmd == "PRIVMSG") {
			handlePrivmsg(client, tokens);
		} else if (cmd == "PART") {
			handlePart(client, tokens);
		} else if (cmd == "MODE") {
			Handle_mode(client, tokens);
		} else if (cmd == "TOPIC") {
			Handle_topic(client, tokens);
		} else {
			sendError(client, "421", cmd + " :Unknown command");
		}
	}
	return true;
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

void Server::Handle_Nick(Client *user, const std::vector<std::string> &tokens)
{
	try {
		if (tokens.size() < 2) {
			sendError(user, "431", ":No nickname given"); // ERR_NONICKNAMEGIVEN
			return;
		}
		std::string Pseudo = tokens[1];
		if (
			!std::isalpha(Pseudo[0]) ||
			Pseudo.find_first_of(" ,:?@!.*#&") != std::string::npos
		) {
			sendError(user, "432", Pseudo + " :Erroneous nickname"); // ERR_ERRONEUSNICKNAME
			return;
		}
		std::map<std::string, Client*>::iterator existing = this->clients.find(Pseudo);
		if (existing != this->clients.end() && existing->second != user) {
			sendError(user, "433", Pseudo + " :Nickname is already in use"); // ERR_NICKNAMEINUSE
			return;
		}
		for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); it++)
		{
			if (it->second == user){
				Client *ptrClient = user;
				clients.erase(it);
				clients[Pseudo] = ptrClient;
				user->setNickname(Pseudo);
				break;
			}
		}
		std::cout << "nombre de clients :" << clients.size() << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Erreur dans Handle_Nick : " << e.what() << std::endl;
		sendError(user, "400", ":Unknown error while handling NICK");
	}
}



void Server::handleUser(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() != 3) {
		sendError(client, "461", "USER :Not enough parameters");
		return;
	}
	if (client->isFullyRegistered()) {
            sendError(client, "462", ":You may not reregister");
            return;
        }

	client->setUsername(tokens[1]);
	client->setRealname(tokens[2]);
	
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
	if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&')) {
		sendError(client, "403", channel_name + " :No such channel");
		return;
	}
	std::map<std::string, Channel*>::iterator it = channels.find(channel_name);
	if (it->second == client->getChannel())
		return ;
	if (it == channels.end())
	{
		if (tokens.size() < 3){
			Channel* new_chan = new Channel(channel_name, client->getFd(), "");
			channels.insert(std::pair<std::string, Channel*>(channel_name, new_chan));
			channel_invite[channel_name] = std::set<int>();
			changeClientChannel(client, new_chan);
			sendMessageWhenJoin(client);
			sendMessageInfoChannel(client);
		} else {
			Channel* new_chan = new Channel(channel_name, client->getFd(), tokens[2]);
			channels.insert(std::pair<std::string, Channel*>(channel_name, new_chan));
			channel_invite[channel_name] = std::set<int>();
			changeClientChannel(client, new_chan);
			sendMessageWhenJoin(client);
			sendMessageInfoChannel(client);
		}
	} else {
		Channel *join_channel = it->second;
		std::map<std::string, std::set<int> >::iterator chan_it = channel_invite.find(join_channel->getName());
		std::set<int>::iterator chan_inv_it = chan_it->second.find(client->getFd());
		if (join_channel->getIsInvitOnly()){
			if (chan_inv_it == chan_it->second.end() && !join_channel->clientOp(client->getFd())){
				sendError(client, "473", channel_name + " :Invite only channel");
				return ;
			}
		}
		int lim = join_channel->getLimitMember();
		std::string pass = join_channel->getPassword();
		int nb_member = join_channel->getMembersSize();
		if (!pass.size()){
			if (lim == -1 || lim > nb_member){
				changeClientChannel(client, it->second);
				sendMessageWhenJoin(client);
				sendMessageInfoChannel(client);
				if (chan_inv_it != chan_it->second.end())
					chan_it->second.erase(chan_inv_it);
			} else {
				sendError(client, "471", channel_name + " :Channel is full");
			}
		} else {
			if (lim == -1 || lim > nb_member) {
				if (tokens.size() != 3){
					sendError(client, "475", channel_name + " :Bad channel key");
				} else {
					if (tokens[2] == it->second->getPassword()){
						changeClientChannel(client, it->second);
						sendMessageWhenJoin(client);
						sendMessageInfoChannel(client);
						if (chan_inv_it != chan_it->second.end())
							chan_it->second.erase(chan_inv_it);
					} else {
						sendError(client, "475", channel_name + " :Bad channel key");
					}
				}
			} else {
				sendError(client, "471", channel_name + " :Channel is full");
			}
		}
	}
}

void Server::handlePrivmsg(Client* client, const std::vector<std::string>& tokens) {
	if (tokens.size() < 2){
		sendError(client, "411", ":No recipient given\r\n");
		return ;
	}
	if (tokens.size() < 3){
		sendError(client, "412", ":No text to send\r\n");
		return ;
	}
	std::string cible = tokens[1];
	if (cible[0] == '#' || cible[0] == '&'){
		std::map<std::string, Channel*>::iterator it = channels.find(cible);
		if (it == channels.end()){
			sendError(client, "403", ":No such channel");
			return ;
		} else {
			if (it->second != client->getChannel()){
				sendError(client, "442", ":Not on the channel");
				return ;
			}
			sendChannelPrivmsg(client, it->second, tokens[2]);
		}
	} else {
		std::map<std::string, Client*>::iterator jt = clients.find(cible);
		if (jt == clients.end()){
			sendError(client, "401", ":No such nick");
			return ;
		} else {
			if (!jt->second->isAuthenticated()){
				sendError(client, "451", ":NOt registered");
				return ;
			}
			sendClientPrivmsg(client, jt->second, tokens[2]);
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
	for (std::map<std::string, Client*>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
		if (it->second->getNickname() == nickname) {
			return (it->second);
		}
	}
	return NULL;
}

// Utilitaires
Client* Server::findClientByFd(int fd) {
	for (std::map<std::string, Client*>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
		if (it->second->getFd() == fd) {
			return (it->second);
		}
	}
	return NULL;
}

bool Server::isNickInUse(const std::string& nickname) {
	for (std::map<std::string, Client*>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
		if (it->second->getNickname() == nickname) {
			return true;
		}
	}
	return false;
}

void Server::sendError(Client* client, const std::string& code, const std::string& message) {
	std::string response = ":ircserv " + code + " " + client->getNickname() + " " + message + "\r\n";
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
	for (std::map<std::string, Client*>::iterator it = clients.begin(); 
		 it != clients.end(); ++it) {
			Channel *current_cl_chan = it->second->getChannel();
			if (current_cl_chan){
				current_cl_chan->unsetMembers(client_fd);
				current_cl_chan->unsetOps(client_fd);
			}
		if (it->second->getFd() == client_fd) {
			removeCLientOnInvite(it->second);
			delete it->second;
			clients.erase(it);
			break;
		}
	}
}

void Server::shutdown() {
	std::cout << "Fermeture du serveur proprement..." << std::endl;

	// Déconnexion de tous les clients
	for (std::vector<int>::iterator it = fds.begin(); it != fds.end(); ++it) {
		close(*it);
	}

	fds.clear();

	// Supprimer tous les clients
	clients.clear();

	// Fermer les sockets principaux
	if (server_fd >= 0)
		close(server_fd);
	if (epoll_fd >= 0)
		close(epoll_fd);
}

void Server::changeClientChannel(Client* client, Channel * channel){
	Channel* cur_client_chan = client->getChannel();
	if (!cur_client_chan){
		client->setChannel(channel);
		channel->setMembers(client->getFd());
	} else {
		if (cur_client_chan->getMembersSize() <= 1){
			std::map<std::string, Channel*>::iterator it = channels.find(cur_client_chan->getName());
			if (it == channels.end()){
				client->setChannel(channel);
				channel->setMembers(client->getFd());
			} else {
				channel_invite.erase(cur_client_chan->getName());
				delete it->second;
				channels.erase(it);
				client->setChannel(channel);
				channel->setMembers(client->getFd());
			}
		} else {
			client->getChannel()->unsetMembers(client->getFd());
			client->setChannel(channel);
			channel->setMembers(client->getFd());
		}
	}
}

void Server::sendMessageWhenJoin(Client* client){
	const std::set<int>& members = client->getChannel()->getmembers();
	std::string join_msg = ":client! " + client->getNickname() + " JOIN " + client->getChannel()->getName() + "\r\n";
	for (std::set<int>::iterator i = members.begin(); i != members.end(); i++)
	{
		if (*i != client->getFd()){
			send(*i, join_msg.c_str(), join_msg.length(), 0);
		}
	}
}

void Server::sendMessageInfoChannel(Client *client){
	const std::set<int>& members = client->getChannel()->getmembers();
	const int& fd_client = client->getFd();
	const std::string& channel_name = client->getChannel()->getName();
	std::string msg = ":ircserv 332 " + client->getNickname() + " " + channel_name + " : " + "TOPIC " + client->getChannel()->getTopic() + "\n";
	send(fd_client, msg.c_str(), msg.length(), 0);
	msg = ":ircserv 353 " + client->getNickname() + " = " + channel_name + " :";
	for (std::set<int>::iterator i = members.begin(); i != members.end(); ++i){
		Client * cl_channel = findClientByFd(*i);
		if (cl_channel){
			if (cl_channel->getFd() != fd_client){
				std::string isOp = client->getChannel()->clientOp(*i) ? "@" : "";
				msg += " " + isOp + cl_channel->getNickname();
			}
		}
	}
	msg += "\n";
	send(fd_client, msg.c_str(), msg.length(), 0);
	msg = ":ircserv 366 " + client->getNickname() + " " + channel_name + " :End of /NAME list\n";
	send(fd_client, msg.c_str(), msg.length(), 0);
}

void Server::sendClientPrivmsg(Client *sender, Client* receiver, std::string msg){
	std::string formt_msg;
	if (msg[0] == ':')
		formt_msg = ":" + sender->getNickname() + " PRIVMSG " + receiver->getNickname() + " " + msg + "\n";
	else
		formt_msg = ":" + sender->getNickname() + " PRIVMSG " + receiver->getNickname() + " :" + msg + "\n";
	send(receiver->getFd(), formt_msg.c_str(), formt_msg.length(), 0);
}

void Server::sendChannelPrivmsg(Client *sender, Channel* chan_receiver, std::string msg){
	std::string formt_msg;
	if (msg[0] == ':')
		formt_msg = ":" + sender->getNickname() + " PRIVMSG " + chan_receiver->getName() + " " + msg + "\n";
	else
		formt_msg = ":" + sender->getNickname() + " PRIVMSG " + chan_receiver->getName() + " " + msg + "\n";
	const std::set<int> members = chan_receiver->getmembers();
	for (std::set<int>::iterator i = members.begin(); i != members.end(); i++){
		if (sender->getFd() != *i){
			send(*i, formt_msg.c_str(), formt_msg.length(), 0);
		}
	}
}

// Exception
Server::ServerErrorException::ServerErrorException(const std::string& msg): msg(msg){}
Server::ServerErrorException::~ServerErrorException() throw(){}
const char *Server::ServerErrorException::what() const throw(){
	return msg.c_str();
}

// void Server::sendToClient(Client& client, const std::string& message) {
//     client.send(message + "\r\n");
// }

// Correction de la fonction MODE (C++98, méthodes Channel existantes, utils externes)
// Doit être déclarée dans Server.hpp : void Handle_mode(Client* client, const std::vector<std::string>& args);
void Server::Handle_mode(Client* client, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendToClient(*client, "461 MODE :Not enough parameters");
        return;
    }
    
    const std::string& target = args[1];
    
    if (target[0] == '#') {
        std::map<std::string, Channel*>::iterator it = channels.find(target);
        if (it == channels.end()) {
            sendToClient(*client, "403 " + target + " :No such channel");
            return;
        }
        
        Channel& channel = *(it->second);
        
        // ✅ Si pas de flags fournis, retourner les modes actuels du channel
        if (args.size() < 3) {
            // Retourner les modes actuels (optionnel selon l'implémentation IRC)
            sendToClient(*client, "324 " + target + " +nt");  // exemple
            return;
        }
        
        if (!channel.clientOp(client->getFd())) {
            sendToClient(*client, "482 " + target + " :You're not channel operator");
            return;
        }
        
        // Maintenant on sait que args[2] existe
        bool adding = true;
        for (size_t i = 0; i < args[2].size(); ++i) {
            char flag = args[2][i];
            switch (flag) {
                case '+': adding = true; break;
                case '-': adding = false; break;
                case 'i': channel.setIsInvitOnly(adding); break; 
				case 't': 
                    channel.setIsRestrictedTopic(adding);  // ✅ NOUVEAU: Mode topic
                    break;
                case 'k':
                    if (args.size() < 4) {  // ✅ Vérifier args[3]
                        sendToClient(*client, "461 MODE :Missing key parameter");
                        return;
                    }
                    if (adding)
                        channel.setPassword(const_cast<std::string&>(args[3]));
                    else {
                        std::string empty = "";
                        channel.setPassword(empty);
                    }
                    break;
                case 'l':
                    if (adding && args.size() >= 4) {  // ✅ Vérifier args[3]
                        std::istringstream iss(args[3]);
                        int lim;
                        if (!(iss >> lim)) {
                            sendToClient(*client, "461 MODE :Invalid limit parameter");
                            return;
                        } else if (lim > 64 || lim < 1) {
							sendToClient(*client, "461 MODE :Invalid limit parameter");
							return ;
						}
                        channel.setLimitMember(lim);
                    } else {
                        channel.setLimitMember(64);
                    }
                    break;
                case 'o':
                    if (args.size() < 4) {  // ✅ Vérifier args[3]
                        sendToClient(*client, "461 MODE :Missing operator parameter");
                        return;
                    }
                    {
                        Client* opClient = findClientByNick(args[3]);
                        if (!opClient) {
                            sendToClient(*client, "401 " + args[3] + " :No such nick");
                            return;
                        }
                        if (adding)
                            channel.setOps(opClient->getFd());
                        else
                            channel.unsetOps(opClient->getFd());
                    }
                    break;
                default:
                    sendToClient(*client, "472 " + std::string(1, flag) + " :is unknown mode");
            }
        }
        broadcastToChannel(channel, ":" + getClientPrefix(*client) + " MODE " + target + " " + args[2]);
    } else {
        sendToClient(*client, "403 " + target + " :No such channel");
    }
}

// Correction de la fonction TOPIC (C++98, méthodes Channel existantes, utils externes)
// Doit être déclarée dans Server.hpp : void Handle_topic(Client* client, const std::vector<std::string>& tokens);
void Server::Handle_topic(Client* client, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        sendError(client, "461", "TOPIC :Not enough parameters");
        return;
    }
    std::string channel_name = tokens[1];
    std::map<std::string, Channel*>::iterator it = channels.find(channel_name);
    if (it == channels.end()) {
        sendError(client, "403", channel_name + " :No such channel");
        return;
    }
    Channel* channel = it->second;
    if (tokens.size() == 2) {
        if (channel->getTopic().empty()) {
            sendReply(client, "331", channel_name + " :No topic is set");
        } else {
            sendReply(client, "332", channel_name + " :" + channel->getTopic());
        }
        return;
    }
    if (channel->getIsRestrictTopic() && !channel->clientOp(client->getFd())) {
        sendError(client, "482", channel_name + " :You're not channel operator");
        return;
    }
    std::string new_topic = tokens[2];
    for (size_t i = 3; i < tokens.size(); ++i) {
        new_topic += " " + tokens[i];
    }
    channel->setTopic(new_topic);
    std::string topic_msg = ":" + client->getPrefix() + " TOPIC " + channel_name + " :" + new_topic;
    broadcastToChannel(*channel, topic_msg);
}

void Server::removeCLientOnInvite(Client *client){
	int client_fd = client->getFd();

	for (std::map<std::string, std::set<int> >::iterator it = channel_invite.begin(); it != channel_invite.end(); it++){
		std::set<int>::iterator it_fd = it->second.find(client_fd);
		if (it_fd != it->second.end()) {
			it->second.erase(it_fd);
		}
	}
	
}