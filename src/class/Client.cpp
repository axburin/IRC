#include "Clients.hpp"

Client::Client(std::string nickname, std::string username, int fd): nickname(nickname), username(username), client_fd(fd){
	client_fd = fd;
}

Client::~Client(void){
}