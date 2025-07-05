#include "CommandParser.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>

IRCMessage CommandParser::parse(const std::string& raw_message) {
    IRCMessage msg;
    
    // Nettoyer le message
    std::string message = sanitizeMessage(raw_message);
    
    if (message.empty()) {
        return msg;
    }
    
    try {
        // 1. Extraire le préfixe (optionnel)
        msg.prefix = extractPrefix(message);
        
        // 2. Extraire la commande (obligatoire)
        msg.command = extractCommand(message);
        if (msg.command.empty()) {
            return IRCMessage();
        }
        
        // 3. Extraire le trailing (avant les paramètres)
        msg.trailing = extractTrailing(message);
        
        // 4. Extraire les paramètres
        msg.parameters = extractParameters(message);
        
        // Validation finale
        if (!msg.isValid()) {
            return IRCMessage();
        }
        
    } catch (const std::exception& e) {
        return IRCMessage();
    }
    
    return msg;
}

std::string CommandParser::sanitizeMessage(const std::string& raw_message) {
    std::string cleaned = raw_message;
    
    // Supprimer \r\n ou \n de fin
    if (cleaned.length() >= 2 && cleaned.substr(cleaned.length() - 2) == "\r\n") {
        cleaned = cleaned.substr(0, cleaned.length() - 2);
    } else if (cleaned.length() >= 1 && cleaned[cleaned.length() - 1] == '\n') {
        cleaned = cleaned.substr(0, cleaned.length() - 1);
    }
    
    // Supprimer \r de fin si présent
    if (cleaned.length() >= 1 && cleaned[cleaned.length() - 1] == '\r') {
        cleaned = cleaned.substr(0, cleaned.length() - 1);
    }
    
    return cleaned;
}

std::string CommandParser::extractPrefix(std::string& message) {
    trimLeading(message);
    
    if (message.empty() || message[0] != ':') {
        return "";
    }
    
    size_t space_pos = message.find(' ');
    if (space_pos == std::string::npos) {
        return "";
    }
    
    std::string prefix = message.substr(1, space_pos - 1);
    message = message.substr(space_pos + 1);
    
    return prefix;
}

std::string CommandParser::extractCommand(std::string& message) {
    trimLeading(message);
    
    if (message.empty()) {
        return "";
    }
    
    size_t space_pos = message.find(' ');
    
    std::string command;
    if (space_pos == std::string::npos) {
        command = message;
        message.clear();
    } else {
        command = message.substr(0, space_pos);
        message = message.substr(space_pos + 1);
    }
    
    return toUpper(command);
}

std::string CommandParser::extractTrailing(std::string& message) {
    size_t trailing_pos = message.find(" :");
    
    if (trailing_pos == std::string::npos) {
        return "";
    }
    
    std::string trailing = message.substr(trailing_pos + 2);
    message = message.substr(0, trailing_pos);
    
    return trailing;
}

std::vector<std::string> CommandParser::extractParameters(std::string& message) {
    std::vector<std::string> parameters;
    
    trimLeading(message);
    
    if (message.empty()) {
        return parameters;
    }
    
    std::string current_param;
    for (size_t i = 0; i < message.length(); i++) {
        char c = message[i];
        
        if (c == ' ') {
            if (!current_param.empty()) {
                parameters.push_back(current_param);
                current_param.clear();
            }
        } else {
            current_param += c;
        }
    }
    
    if (!current_param.empty()) {
        parameters.push_back(current_param);
    }
    
    return parameters;
}

void CommandParser::trimLeading(std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        str.clear();
    } else {
        str = str.substr(start);
    }
}

std::string CommandParser::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool CommandParser::isValidCommand(const std::string& command) {
    if (command.empty() || command.length() > 20) {
        return false;
    }
    
    bool all_alpha = true;
    bool all_digit = true;
    
    for (size_t i = 0; i < command.length(); i++) {
        if (!std::isalpha(command[i])) all_alpha = false;
        if (!std::isdigit(command[i])) all_digit = false;
    }
    
    return all_alpha || all_digit;
}

bool CommandParser::isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }
    
    char first = nickname[0];
    if (!std::isalpha(first) && first != '[' && first != ']' && 
        first != '{' && first != '}' && first != '\\' && first != '|' && 
        first != '^' && first != '_' && first != '`') {
        return false;
    }
    
    for (size_t i = 1; i < nickname.length(); i++) {
        char c = nickname[i];
        if (!std::isalnum(c) && c != '[' && c != ']' && c != '{' && c != '}' && 
            c != '\\' && c != '|' && c != '^' && c != '_' && c != '`' && c != '-') {
            return false;
        }
    }
    
    return true;
}

bool CommandParser::isValidChannelName(const std::string& channel) {
    if (channel.empty() || channel.length() > 50) {
        return false;
    }
    
    if (channel[0] != '#' && channel[0] != '&') {
        return false;
    }
    
    for (size_t i = 1; i < channel.length(); i++) {
        char c = channel[i];
        if (c == ' ' || c == ',' || c == '\r' || c == '\n' || c == '\0') {
            return false;
        }
    }
    
    return true;
}

// MessageBuffer Implementation
bool MessageBuffer::addData(const std::string& data) {
    if (buffer.length() + data.length() > MAX_BUFFER_SIZE) {
        return false;
    }
    
    buffer += data;
    return true;
}

std::vector<IRCMessage> MessageBuffer::extractMessages() {
    std::vector<IRCMessage> messages;
    
    size_t pos = 0;
    while ((pos = buffer.find("\r\n")) != std::string::npos || 
           (pos = buffer.find("\n")) != std::string::npos) {
        
        std::string raw_message = buffer.substr(0, pos);
        
        size_t separator_len = (buffer[pos] == '\r' && pos + 1 < buffer.length() && 
                               buffer[pos + 1] == '\n') ? 2 : 1;
        
        buffer.erase(0, pos + separator_len);
        
        if (!raw_message.empty()) {
            IRCMessage msg = CommandParser::parse(raw_message);
            if (msg.isValid()) {
                messages.push_back(msg);
            }
        }
    }
    
    return messages;
}

void MessageBuffer::clear() {
    buffer.clear();
}

bool MessageBuffer::isEmpty() const {
    return buffer.empty();
}

size_t MessageBuffer::getBufferSize() const {
    return buffer.length();
}