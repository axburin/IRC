#include "error.hpp"
#include "Server.hpp"
#include <iostream>

void errorManager(int flags, Server*){
	switch (flags)
	{
	case NO_START_WITH_ZERO_PORT:
		std::cerr << "Error : <port> no number 0 on first" << std::endl;
		break;
	case NON_DIGIT_PORT:
		std::cerr << "Error : <port> need to be a digit" << std::endl;
		break;
	case TOO_LARGE_NUMBER_PORT:
		std::cerr << "Error : <port> too large number" <<std::endl;
		break;
	case EMPTY_PORT:
		std::cerr << "Error : <port> cannot be empty" << std::endl;
		break;
	case INCORRECT_LENGTH_PASSWORD:
		std::cerr << "Error : <password> length between 6 & 12 characters included" << std::endl;
		break;
	case ONLY_VISIBLE_CHAR_PASSWORD:
		std::cerr << "Error : <password> can contain only alphanumerical and special chraracter no non-visible character" << std::endl;
		break;
	default:
		std::cerr << "Error : unknow" << std::endl;
		break;
	}
}