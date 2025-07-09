#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>


class Channel {
	private :
		std::set<int> invited;
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

		std::string getName(void)const;
		std::set<int> getmembers(void)const;
		std::set<int> getOps(void) const;
		int getLimitMember(void) const;
		bool getIsInvitOnly(void) const;
		bool getIsRestrictTopic(void) const;
		std::string getTopic(void) const;
		std::string getPassword(void)const;
		int getMembersSize(void)const;

		void setName(std::string& name);
		void setMembers(int fd);
		void setOps(int fd);
		void setLimitMember(int nb);
		void setIsInvitOnly(bool bl);
		void setIsRestrictedTopic(bool bl);
		void setTopic(std::string& topic);
		void setPassword(std::string& password);

		void unsetMembers(int fd);
		void unsetOps(int fd);
 
		bool findClientInChannel(int client_fd);
		bool clientOp(int client_op);

};

#endif