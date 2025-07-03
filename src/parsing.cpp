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

bool    parsing(std::string buffer) // ? changer en int pour le code d'erreur
{
    if (buffer.empty())
        return (false);

    std::istringstream iss(buffer);
    std::string cmd;
    std::getline(iss, cmd, ' ');

    if (cmd.empty())
        return (false);
    else
    {
        std::string param;
        std::getline (iss, param);

        std::string validcmd[] = { "JOIN", "NICK", "MODE", "INVITE", "TOPIC", "KICK" }; // ? op function's
        void (*function[])(std::string) = { &join, &nick, &mode, &invite, &topic, &kick };
        for (int i = 0; i < 6; i++)
        {
            if (validcmd[i] == cmd)
            {
                function[i];
                return (true);
            }
        }
    }
    return (false);
}