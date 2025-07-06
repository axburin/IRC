# include "Channel.hpp"
# include "Server.hpp"
# include "Clients.hpp"
# include "error.hpp"


Channel::Channel(std::string name, int op, std::string password): name(name), password(password){
	members.insert(op);
	ops.insert(op);
	limit_member = 64;
	is_invit_only = false;
	is_restrict_topic = false;
	topic = "";
}

Channel::~Channel(void){
}

bool	Channel::findClientInChannel(int client_fd)
{
	std::set<int>::iterator it = members.find(client_fd);

	if (it != members.end())
		return (true);
	else
		return (false);
}

bool	Channel::clientOp(int client_fd)
{
	std::set<int>::iterator it = ops.find(client_fd);

	if (it != members.end())
		return (true);
	else
		return (false);
}