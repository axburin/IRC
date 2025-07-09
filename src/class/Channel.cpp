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

std::string Channel::getName(void)const {
	return (name);
}

std::set<int> Channel::getmembers(void)const {
	return (members);
}

std::set<int> Channel::getOps(void)const {
	return (ops);
}

int Channel::getLimitMember(void)const {
	return (limit_member);
}

bool Channel::getIsInvitOnly(void)const {
	return (is_invit_only);
}

bool Channel::getIsRestrictTopic(void)const {
	return (is_restrict_topic);
}

std::string Channel::getTopic(void) const {
	return (topic);
}

std::string Channel::getPassword(void) const {
	return (password);
}

int Channel::getMembersSize(void)const {
	return (members.size());
}

void Channel::setName(std::string& name){
	this->name = name;
}

void Channel::setMembers(int fd){
	std::set<int>::iterator it = members.find(fd);
	if (it == members.end())
		members.insert(fd);
}

void Channel::setOps(int fd){
	std::set<int>::iterator it = ops.find(fd);
	if (it == ops.end())
		ops.insert(fd);
}

void Channel::setLimitMember(int nb){
	// can't exceed 64;
	limit_member = nb;
}

void Channel::setIsInvitOnly(bool bl){
	is_invit_only = bl;
}

void Channel::setIsRestrictedTopic(bool bl){
	is_restrict_topic = bl;
}

void Channel::setTopic(std::string& topic){
	this->topic = topic;
}

void Channel::setPassword(std::string& password){
	this->password = password;
}

void Channel::unsetMembers(int fd){
	std::set<int>::iterator it = members.find(fd);
	if (it == members.end())
		return ;
	else
		members.erase(it);
}

void Channel::unsetOps(int fd){
	std::set<int>::iterator it = ops.find(fd);
	if (it == ops.end())
		return ;
	else
		ops.erase(it);
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

	if (it != ops.end())
		return (true);
	else
		return (false);
}