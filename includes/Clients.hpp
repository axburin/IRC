#pragma once

#include <string>
#include <map>

class Client{
	public:
		Client(int fd, const std::string& ip = "", int port = 0);
		~Client();
		int getFd() const { return _fd; }
		const std::string& getIp() const { return _ip; }
		int getPort() const { return _port; }
		std::string& getNickname() { return _nickname; }
		std::string& getBuffer() { return _buffer; }
		bool isAuthenticated() const { return _authenticated; }
		void setAuthenticated(bool auth) { _authenticated = auth; }
	private:
		int _fd;
		std::string _ip;
		int _port;
		std::string _nickname;
		std::string _buffer;
		bool _authenticated;
};

class Gestion_Client{
	public :
	std::map <int, Client*> clients;
};