#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <vector>

struct IRCMessage {
    std::string prefix;                    // Sans le ':' initial
    std::string command;                   // Commande (NICK, JOIN, etc.)
    std::vector<std::string> parameters;   // Paramètres
    std::string trailing;                  // Texte après ':'
    
    // Constructeurs
    IRCMessage();
    IRCMessage(const std::string& cmd);
    
    // Méthodes utilitaires
    bool isEmpty() const;
    bool hasPrefix() const;
    bool hasTrailing() const;
    size_t getParameterCount() const;
    std::string getParameter(size_t index) const;
    bool isValid() const;
    
    // Debug
    std::string toString() const;
};

#endif