# include "Server.hpp"
# include "Channel.hpp"
# include "Clients.hpp"
# include "error.hpp"

Channel*	Server::findChannelByName(std::string Name) // move to server.cpp
{
	std::map<std::string, Channel>::iterator it = channels.find(Name);
	return &(it->second);
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
	if (!channel)
		return; // ! what error ?

	std::string kickedNick = tokens[2];
	if (kickedNick.empty()) {
		sendError(client, "403", kickedNick + " :empty Nickname to kick"); // ! error to check
		return;
	}

	Client* kickedClient = findClientByNick(kickedNick);
	if (!kickedClient) {
		sendError(client, "404", kickedNick + " :does not exist in channel " + channel_name);
		return;
	}

	if (!channel->clientOp(client->getFd()) && !channel->findClientInChannel(kickedClient->getFd()))
	{

	}
}

