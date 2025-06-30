#ifndef SOCKADDR_UTILS_HPP
#define SOCKADDR_UTILS_HPP

#include <netinet/in.h>
#include <sys/socket.h>

// Alias pour simplifier l'utilisation de sockaddr_in et sockaddr
typedef struct sockaddr_in t_sockaddr_in;
typedef struct sockaddr t_sockaddr;

typedef socklen_t t_socklen;

#endif
