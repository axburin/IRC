#include "error.hpp"
#include <fcntl.h>
#include <cctype>
#include <sstream>
#include <cctype>
#include <algorithm>
// Fonction utilitaire pour convertir int en string (C++98)
std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Parser simple temporaire
std::vector<std::string> simpleParse(const std::string& command) {
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::string token;
    
    // Parser mot par mot jusqu'à rencontrer ":"
    while (iss >> token) {
        if (!token.empty() && token[0] == ':') {
            // Tout le reste fait partie du message
            tokens.push_back(token);
            std::string remaining;
            std::getline(iss, remaining);
            if (!remaining.empty()) {
                tokens.back() += remaining;
            }
            break;
        } else {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

void setNonBlocking(int fd){
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