#include "../includes/createServerSocket.hpp"
#include "../includes/parsing.hpp"
#include "../includes/Clients.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return (1);
	}

	int port = parsingPort(argv[1]);
	if (port == -1)
		return (1);
	std::string password = argv[2];

	int sock_fd = createSocketserver(port);
	if (sock_fd == -1)
		return (1);

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		std::cerr << "Error: Failed to create epoll instance" << std::endl;
		close(sock_fd);
		return (1);
	}

	const int MAX_EVENTS = 64;
	struct epoll_event events[MAX_EVENTS];

	std::map<int, Client*> Clients;
	char buffer[512];

	while(true)
	{
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		for(int i = 0; i < n; ++i)
		{
			if(events[i].data.fd == sock_fd)
			{
				// Nouvelle connexion à accepter
				int client_fd = accept_new_client(sock_fd, epoll_fd);
				if(client_fd == -1){
					std::cout << "Error : impossible to accept a new client" << std::endl;
					continue;
				}
				Clients[client_fd] = new Client(client_fd, "0.0.0.0", port);
				std::string welcome = ":ircserv 001 Welcome to the IRC server\r\n";
    			send(client_fd, welcome.c_str(), welcome.size(), 0);
			}
			else
			{
				int fd = events[i].data.fd; //savoir sur quel socket il y a un événement à traiter.
				ssize_t bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
				if(bytes_read <= 0)
				{
					close(fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
					delete Clients[fd];
					Clients.erase(fd);
					continue;
				}
				buffer[bytes_read] = '\0';
				std::cout << "Messag from" << fd << ":" << buffer << std::endl;
				// parser les message irc
			}
		}
	}
		for (std::map<int, Client*>::iterator it = Clients.begin(); it != Clients.end(); ++it) {
        delete it->second;
	}
	Clients.clear();
	close(sock_fd);
	close(epoll_fd);
	return(0);
}
