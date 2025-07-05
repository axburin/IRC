#include "IRCMessage.hpp"
#include <sstream>

IRCMessage::IRCMessage() {}

IRCMessage::IRCMessage(const std::string& cmd) : command(cmd) {}

bool IRCMessage::isEmpty() const {
    return command.empty();
}

bool IRCMessage::hasPrefix() const {
    return !prefix.empty();
}

bool IRCMessage::hasTrailing() const {
    return !trailing.empty();
}

size_t IRCMessage::getParameterCount() const {
    return parameters.size();
}

std::string IRCMessage::getParameter(size_t index) const {
    if (index < parameters.size()) {
        return parameters[index];
    }
    return "";
}

bool IRCMessage::isValid() const {
    if (command.empty()) {
        return false;
    }
    
    // Vérifier que la commande ne contient pas d'espaces
    if (command.find(' ') != std::string::npos) {
        return false;
    }
    
    // Vérifier les paramètres
    for (size_t i = 0; i < parameters.size(); i++) {
        const std::string& param = parameters[i];
        if (param.find('\r') != std::string::npos || 
            param.find('\n') != std::string::npos ||
            param.find('\0') != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

std::string IRCMessage::toString() const {
    std::ostringstream oss;
    
    if (hasPrefix()) {
        oss << ":" << prefix << " ";
    }
    
    oss << command;
    
    for (size_t i = 0; i < parameters.size(); i++) {
        oss << " " << parameters[i];
    }
    
    if (hasTrailing()) {
        oss << " :" << trailing;
    }
    
    return oss.str();
}