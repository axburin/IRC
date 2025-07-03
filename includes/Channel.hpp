#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>


class Channel {
	private :
		std::string name;
		std::set<int> members;
		std::set<int> ops;
		int limit_member; // -1 pas de limite | 0 impossible | 64 max
		bool is_invit_only; // default false
		bool is_restrict_topic;
		std::string topic;
		std::string password;

	public :
		Channel(std::string name, int op, std::string password);
		~Channel(void);
};

#endif