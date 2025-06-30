#include "../includes/parsing.hpp"
#include <iostream>
#include <cctype>
#include <cstring>
#include <cstdlib>

int parsingPort(char *argv)
{
    for(size_t i = 0; i < strlen(argv); i++)
    {
        if(!isdigit(argv[i])) {
            std::cout << "Error : port not correct" << std::endl;
            return (-1);
        }
    }
    int port = atoi(argv);
    if(port <= 0 || port > 65535)
    {
        std::cout << "Error : port out of range" << std::endl;
        return (-1);
    }
    return(port);
}