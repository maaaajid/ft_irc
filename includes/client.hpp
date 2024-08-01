#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "irc.hpp"

using namespace std;
class client
{
    private :
        int     c_fd;//client socket
        string  nickname;
        string  username;
        string  c_ip;
        string  c_state; // client connection state

    public :
        client(){};
        ~client(){};
        void            setC_fd(int fd){this->c_fd = fd;};
        void            setNickname(string nickName){this->nickname = nickName;};
        void            setUsername(string userName){this->username = userName;};
        void            setC_ip(string c_Ip){this->c_ip = c_Ip;};
        void            setC_state(string c_State){this->c_state = c_State;};
        int             getC_fd(void){return(this->c_fd);};
        string     getnickName(void){return (this->nickname);};
        string     getuserName(void){return (this->username);};
        string     getC_ip(void){return (this->c_ip);};
        string     getC_state(void){return (this->c_state);};

};

#endif