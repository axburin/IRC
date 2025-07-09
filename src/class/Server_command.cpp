# include "Server.hpp"
# include "Channel.hpp"
# include "Clients.hpp"
# include "error.hpp"

Channel*	Server::findChannelByName(std::string Name) // need to be moved to server.cpp
{
	std::map<std::string, Channel *>::iterator it = channels.find(Name);
	if (it != channels.end())
		return (it->second);
	else
		return(NULL);
}


void	Server::KICK(Client* client, Channel* channel, const std::vector<std::string>& tokens)
{
	if (!channel->clientOp(client->getFd())) {
		sendError(client, "482", channel->getName() + " :You're not channel operator");
		return;
	}
		
	std::istringstream Nicks(tokens[2]);
	std::string kickedNick;
	while (std::getline(Nicks, kickedNick, ',')) {
		if (kickedNick.empty()) {
			sendError(client, "431", ":No nickname given");
			continue;
		}
		
		Client* kickedClient = findClientByNick(kickedNick);
		if (!kickedClient) {
			sendError(client, "401", kickedNick + " :does not exist");
			continue;
		}
		
		if (!channel->findClientInChannel(kickedClient->getFd())) {
			sendError(client, "441", kickedNick + " " + channel->getName() + " :They aren't on that channel");
			continue;
		}
		
		if (!channel->findClientInChannel(client->getFd()))
		{
			sendError(client, "442", channel->getName() + " :You're not on that channel");
			continue;
		}
		
		std::string reason = "No reason given";
		if (tokens.size() >= 4) {
			reason = tokens[3];
			if (reason[0] == ':') {
				reason = reason.substr(1);
			}
			for (size_t i = 4; i < tokens.size(); i++) {
				reason += " " + tokens[i];
			}
		}
		std::string kickMsg = ":" + client->getNickname() + " KICK " + channel->getName() + " " + kickedNick + " :" + reason + "\r\n";
		const std::set<int> members = channel->getmembers();
		
		for (std::set<int>::iterator i = members.begin(); i != members.end(); i++) {
			send(*i, kickMsg.c_str(), kickMsg.length(), 0);
			channel->unsetMembers(kickedClient->getFd());
		}
		
		kickedClient->setChannel(NULL);
		if (channel->clientOp(kickedClient->getFd())) {
			channel->unsetOps(kickedClient->getFd());
		}
	}
}

void	Server::handleKick(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3) {
		sendError(client, "431", ":No nickname given");
		return;
	}

	std::istringstream channels(tokens[1]);
	std::string channel_name;

	while (std::getline(channels, channel_name, ',')) {
		if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&')) {
			sendError(client, "403", channel_name + " :No such channel"); 
			continue;
		}
		Channel* channel = findChannelByName(channel_name);
		if (!channel) {
			sendError(client, "403", channel_name + " :No such channel");
			continue;
		}
		KICK(client, channel, tokens);
	}
}

void	Server::handleInvite(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3) {
		sendError(client, "461", "INVITE :Need more params");
		return;
	}

	std::string nick = tokens[1];
	// if (nick.empty()) {
	// 	sendError(client, "431", ":No nickname given");
	// 	return;
	// }
	
	Client* invitedClient = findClientByNick(nick);
	if (!invitedClient) {
		sendError(client, "401", nick + " :No such nick");
		return;
	}

	std::string channel_name = tokens[2];
	if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&')) {
		sendError(client, "403", channel_name + " :No such channel"); 
		return;
	}
	
	Channel* channel = findChannelByName(channel_name);
	if (!channel) {
		sendError(client, "403", channel_name + " :No such channel");
		return;
	}
	
	if (!channel->clientOp(client->getFd())) {
		sendError(client, "482", channel_name + " :You're not channel operator");
		return;
	}

	if (client->getChannel()->getName() != channel->getName()) {
		sendError(client, "442", channel_name + " :Not on that channel");
		return;
	}

	// if (!channel->findClientInChannel(client->getFd())) {
	// 	sendError(client, "442", channel_name + " :You're not on that channel");
	// 	return;
	// }

	if (channel->findClientInChannel(invitedClient->getFd())) {
		sendError(client, "443", nick + " " + channel_name + " :User on channel");
		return;
	}


	sendReply(client, "341", nick + " " + channel_name);

	std::string inviteMsg = ":" + client->getNickname() + "!" + client->getUsername() + " INVITE " + nick + " " + channel_name + "\r\n";
	send(invitedClient->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);
	if (channel->getIsInvitOnly()){
		std::map<std::string, std::set<int> >::iterator it = channel_invite.find(channel->getName());
		if (it == channel_invite.end()){
			channel_invite[channel->getName()].insert(invitedClient->getFd());
		} else {
			it->second.insert(invitedClient->getFd());
		}
	}
}