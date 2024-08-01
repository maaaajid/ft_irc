#ifndef SERVER_HPP
# define SERVER_HPP

# include "irc.hpp"

class Server
{
    private :
        int             ServerPort;
        string          ServerPassword;
        string          ServerIP;
        int             socketfd;
        int             epollFd;
        map<int, client> clients;
    
    public :
        Server(Parse par);
        void    serverSockCreate();
        void    startCommunication();
        void    createNewConnection();
        ~Server();
};

#endif