#include <iostream>
#include <cstdlib>
#include "error.hpp"
#include "Server.hpp"
#include <csignal>

Server* g_server = NULL;

void handle_sigint(int) {
	if (g_server) {
		g_server->stop();
	}
}

int main(int argc, char *argv[]){
	if (argc != 3){
		std::cerr << "Error invalid number of arguments ./ircserv <port> <password>" << std::endl;
	} else {
		int flag = strIsDigit(argv[1]);
		int port;
		if (flag){
			 errorManager(flag, NULL);
			 return (1);
		}
		port = std::atoi(argv[1]);
		if (port < 1 || port  > 65535){
			errorManager(TOO_LARGE_NUMBER_PORT, NULL);
			return (1);
		}
		flag = checkPassword(argv[2]);
		if (flag){
			errorManager(flag, NULL);
			return (1);
		}
		std::string password(argv[2]);
		std::cout << "all good" <<std::endl;
		std::signal(SIGINT, handle_sigint);
		try {
			Server* ircServer = new Server(port, password);
			g_server = ircServer;
			ircServer->listenServer();
		} catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	}
	return (0);
}