#ifndef CREATE_SERVER_SOCKET_HPP
#define CREATE_SERVER_SOCKET_HPP

int createSocketserver(int port);
int accept_new_client(int sock_fd, int epoll_fd);

#endif
