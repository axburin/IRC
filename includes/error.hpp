#ifndef ERROR_HPP
# define ERROR_HPP

#define NON_DIGIT_PORT 2
#define TOO_LARGE_NUMBER_PORT 3
#define NO_START_WITH_ZERO_PORT 4
#define EMPTY_PORT 5
#define INCORRECT_LENGTH_PASSWORD 6
#define ONLY_VISIBLE_CHAR_PASSWORD 7

#include <string>
#include <vector>

class Server;

std::string intToString(int value);
std::vector<std::string> simpleParse(const std::string& command);
std::string toUpper(const std::string& str);                                                 
void errorManager(int flags, Server*);
void setNonBlocking(int fd);
int strIsDigit(char *str);
int checkPassword(char* str);

#endif