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

Server::Server(int port, std::string password ): password(password){
	sockaddr_in server_addr;
	epoll_event event;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		throw ServerErrorException("Socket error");
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
					std::cout << "Nouveau client connecte : fd = " << client_fd << std::endl;
				}
			} else {
				char buffer[1024];
				int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
				if (bytes_read <= 0) {
					std::cout << "Client deconnecte: fd = " << events[i].data.fd << std::endl;
					close(events[i].data.fd);
				} else {
					buffer[bytes_read] = '\0';
					std::string message(buffer);
					if (message == "/QUIT\n"){
						std::cout << "fin du serveur" << std::endl;
						return ;
					}
					std::cout << "Recu de fd " << events[i].data.fd << ": " << buffer;
					send(events[i].data.fd, buffer, bytes_read, 0);
				}
			}
		}
	}
}

Server::ServerErrorException::ServerErrorException(const std::string& msg): msg(msg){
}

Server::ServerErrorException::~ServerErrorException() throw(){
}

const char *Server::ServerErrorException::what() const throw(){
	return msg.c_str();
}