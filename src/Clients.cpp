#include "../includes/Clients.hpp"
#include <string>
#include <cstring>
#include <unistd.h>

Client::Client(int fd, const std::string& ip, int port)
    : _fd(fd), _ip(ip), _port(port), _nickname(""), _buffer(""), _authenticated(false) {}

Client::~Client() {
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
}

// Optionally, add more methods for Client if needed.
