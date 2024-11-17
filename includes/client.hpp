#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "irc.hpp"

#define MAX_READ_ONCE 1024

//client states
# define WAITING_AUTH 0
# define AUTHENTICATED 1

using namespace std;
class Client
{
    private :
        int     c_fd; //client socket
        bool    auth;
		bool	passValid;
        bool    isOperator;
        string  nickname;
        string  username;
        string  c_ip;
        int     c_state; // client connection state

    public :
        Client();
        ~Client();
        void        setC_fd(int fd);
        void        setNickname(string nickName);
        void        setUsername(string userName);
        void        setC_ip(string c_Ip);
        void        setC_state(int c_State);
        void        setAuth(bool auth);
        void        setOperator(bool op);
        int         getC_fd(void) const;
        string      getnickName(void);
        string      getuserName(void);
        string      getC_ip(void);
        int         getC_state(void);
		bool	    getAuth(void);
        bool        getIsOperator();
        void        sendMessage(const std::string &message);

		bool        getPassValid(void);
		void        setPassValid(bool valid);
};

#endif