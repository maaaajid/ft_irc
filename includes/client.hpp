#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "irc.hpp"

#define MAX_READ_ONCE 1024

using namespace std;
class Client
{
    private :
        int     c_fd;//client socket
        bool    auth;
		bool	passValid;
        string  nickname;
        string  username;
        string  c_ip;
        string  c_state; // client connection state

    public :
        Client();
        ~Client();
        void            setC_fd(int fd);
        void            setNickname(string nickName);
        void            setUsername(string userName);
        void            setC_ip(string c_Ip);
        void            setC_state(string c_State);
        void	setAuth(bool auth);
        int             getC_fd(void);
        string     getnickName(void);
        string     getuserName(void);
        string     getC_ip(void);
        string     getC_state(void);
		bool	getAuth(void);


		bool getPassValid(void)
		{
			return (this->passValid);
		}
		void	setPassValid(bool valid)
		{
			this->passValid = valid;
		}

};

#endif