#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "irc.hpp"

#define MAX_READ_ONCE 1024

// had states a baker khass ywliw conncted, disconnected.
//client states
# define WAITING_AUTH 0
# define AUTHENTICATED 1

using namespace std;
class Server;
class Client
{
    private :
        int     c_fd; //client socket
        bool    isOperator;
        std::string  nickname;
        std::string  username;
        std::string  c_ip;
        int     c_state; // client connection state

        //for Auth the user
        bool    auth;
        bool    passValid;
        bool    nickValid;
        bool    userValid;

    public :
        Client();
        ~Client();
        void        setC_fd(int fd);
        void        setNickname(string nickName);
        void        setUsername(string userName);
        void        setC_ip(string c_Ip);
        void        setC_state(int c_State);
        void        setOperator(bool op);
        int         getC_fd(void) const;
        string      getnickName(void);
        string      getuserName(void);
        string      getC_ip(void);
        int         getC_state(void);
        bool        getIsOperator();
        void        sendMessage(const std::string &message);



         //for Auth the user
		bool	    getAuth(void);
        void        setAuth(bool auth);
		bool        getPassValid(void);
        bool       getNickValid(void);
        bool      getUserValid(void);

        void        setPassValid(bool valid);

        //cmds handlers for Auth
        void        passHandler(std::vector<std::string> &commands, Server &server);
        void        nickHandler(std::vector<std::string> &commands);
        void        userHandler(std::vector<std::string> &commands);
        // void        quitHandler(std::vector<std::string> &commands);



};
bool isValidNick(const std::string& nick);

#endif