#ifndef IRC_UTILS_HPP
#define IRC_UTILS_HPP

#include <string>
class Client;
class Channel;

void sendToClient(Client& client, const std::string& message);
void broadcastToChannel(Channel& channel, const std::string& message);
// std::string toUpper(const std::string& str);
std::string getClientPrefix(const Client& client);
std::string intToString(int value);

#endif
