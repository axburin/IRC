#include "Channel.hpp"

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