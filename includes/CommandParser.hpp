#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include "IRCMessage.hpp"
#include <string>
#include <vector>

class CommandParser {
public:
    // Méthode principale de parsing
    static IRCMessage parse(const std::string& raw_message);
    
    // Méthodes utilitaires
    static std::string sanitizeMessage(const std::string& raw_message);
    static bool isValidCommand(const std::string& command);
    static bool isValidNickname(const std::string& nickname);
    static bool isValidChannelName(const std::string& channel);
    
private:
    // Méthodes internes
    static std::string extractPrefix(std::string& message);
    static std::string extractCommand(std::string& message);
    static std::string extractTrailing(std::string& message);
    static std::vector<std::string> extractParameters(std::string& message);
    
    // Utilitaires
    static void trimLeading(std::string& str);
    static std::string toUpper(const std::string& str);
};

// Buffer pour messages partiels
class MessageBuffer {
private:
    std::string buffer;
    static const size_t MAX_BUFFER_SIZE = 4096;
    
public:
    bool addData(const std::string& data);
    std::vector<IRCMessage> extractMessages();
    void clear();
    bool isEmpty() const;
    size_t getBufferSize() const;
};

#endif