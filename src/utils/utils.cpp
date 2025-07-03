#include "error.hpp"
#include <fcntl.h>
#include <cctype>

void setNonBLocking(int fd){
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int strIsDigit(char *str){
	int i = 0;
	if (str[0] == '0')
		return NO_START_WITH_ZERO_PORT;
	while (str[i]){
		if (!isdigit(str[i]))
			return NON_DIGIT_PORT;
		++i;
	}
	if (i > 5)
		return TOO_LARGE_NUMBER_PORT;
	if (i == 0)
		return EMPTY_PORT;
	return (0);
}

int checkPassword(char * str){
	int i = 0;
	while (str[i]){
		if (str[i] <= 32 || str[i] >= 127)
			return ONLY_VISIBLE_CHAR_PASSWORD;
		++i;
	}
	if (i < 6 || i > 12)
		return INCORRECT_LENGTH_PASSWORD;
	return (0);
}