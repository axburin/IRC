#include "../includes/createServerSocket.hpp"
#include "../includes/sockaddr_utils.hpp"
#include "../includes/Clients.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <sys/epoll.h>

int createSocketserver(int port)
{
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	t_sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; //Pv4
	addr.sin_addr.s_addr = INADDR_ANY; // ip
	addr.sin_port = htons(port); // port

	if(bind(sock_fd, (t_sockaddr*)&addr, sizeof(addr)) == -1)
	{
		std::cout << "Error : Failed to bind socket to port" << std::endl;
		close(sock_fd);
		return (-1);
	}
	if(listen(sock_fd, SOMAXCONN) == -1)
	{
		std::cout << "Error : Failed to listen on socket" << std::endl;
		close(sock_fd);
		return (-1);
	}
	if(fcntl(sock_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cout << "Error : Failed to set socket to non-blocking mode" << std::endl;
		close(sock_fd);
		return (-1);
	}
	int epoll_fd = epoll_create1(0);
	if(epoll_fd == -1)
	{
		std::cout << "Error : Failed to create epoll instance" << std::endl;
		close(sock_fd);
		close(epoll_fd);
		return (-1);
	}
	// ajout du socket a epoll pour le surveiller
	struct epoll_event server_event;
	server_event.events = EPOLLIN;// Surveiller les événements de lecture (nouvelles connexions)
	server_event.data.fd = sock_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &server_event) == -1) {
    	std::cout << "Error : Failed to add server socket to epoll" << std::endl;
    	close(sock_fd);
    	close(epoll_fd);
    	return 1;
	}

	return sock_fd;
}

int accept_new_client(int sock_fd, int epoll_fd) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int client_fd = accept(sock_fd, (struct sockaddr *)&addr, &addrlen);
    if (client_fd == -1) {
        std::cerr << "Error: Failed to accept connection" << std::endl;
        return -1;
    }
    // Mettre le client en non bloquant
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error: Failed to set client socket to non-blocking mode" << std::endl;
        close(client_fd);
        return -1;
    }
    // Ajouter le client à epoll
    struct epoll_event client_event;
    client_event.events = EPOLLIN;
    client_event.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
        std::cerr << "Error: Failed to add client socket to epoll" << std::endl;
        close(client_fd);
        return -1;
    }

    std::cout << "New client connected: fd=" << client_fd << std::endl;
    return client_fd;
}