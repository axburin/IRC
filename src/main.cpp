#include <iostream>
#include <cstdlib>
#include "error.hpp"

int main(int argc, char *argv[]){
	if (argc != 3){
		std::cerr << "Error invalid number of arguments ./irsserv <port> <password>" << std::endl;
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
	}
	return (0);
}