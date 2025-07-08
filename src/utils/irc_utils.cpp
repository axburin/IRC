// filepath: /home/axburin-/Desktop/irclvv/src/utils/utils.cpp
#include <string>
#include <set>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../../includes/Clients.hpp"
#include "../../includes/Channel.hpp"

// Envoi un message à un client
void sendToClient(Client& client, const std::string& message) {
    std::string formattedMessage = message + "\r\n";
    send(client.getFd(), formattedMessage.c_str(), formattedMessage.length(), 0);
}

// Broadcast à tous les membres d'un channel
void broadcastToChannel(Channel& channel, const std::string& message) {
    const std::set<int>& members = channel.getmembers();
    std::string msg = message + "\r\n";
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
}

// upper-case une string
std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// obtenir le prefix IRC d'un client
std::string getClientPrefix(const Client& client) {
    return client.getNickname() + "!" + client.getUsername() + "@localhost";
}

// Utilitaire pour convertir int en string
std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
