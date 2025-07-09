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

void	Server::handleKick(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3) {
		sendError(client, "431", ":No nickname given");
		return;
	}

	std::string channel_name = tokens[1];

	if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&')) {
		sendError(client, "403", channel_name + " :No such channel"); 
		return;
	}

	Channel* channel = findChannelByName(channel_name);
	if (!channel) {
		sendError(client, "403", channel_name + " :No such channel");
		return;
	}

	std::string kickedNick = tokens[2];
	if (kickedNick.empty()) {
		sendError(client, "431", ":No nickname given");
		return;
	}

	Client* kickedClient = findClientByNick(kickedNick);
	if (!kickedClient) {
		sendError(client, "401", kickedNick + " :does not exist");
		return;
	}

	if (!channel->findClientInChannel(kickedClient->getFd())) {
		sendError(client, "441", kickedNick + " " + channel_name + " :They aren't on that channel");
		return;
	}

	if (!channel->findClientInChannel(client->getFd()))
	{
		sendError(client, "442", channel_name + " :You're not on that channel");
		return;
	}

	if (!channel->clientOp(client->getFd())) {
		sendError(client, "482", channel_name + " :You're not channel operator");
		return;
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
	channel->unsetMembers(kickedClient->getFd());
	std::string kickMsg = ":" + client->getNickname() + " KICK " + channel_name + " " + kickedNick + " :" + reason + "\r\n";
	const std::set<int> members = channel->getmembers();

    for (std::set<int>::iterator i = members.begin(); i != members.end(); i++) {
        send(*i, kickMsg.c_str(), kickMsg.length(), 0);
    }
    
    kickedClient->setChannel(NULL);
	if (channel->clientOp(kickedClient->getFd())) {
		channel->unsetOps(kickedClient->getFd());
	}
}

