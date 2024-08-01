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
        vector<client>  usersList;
    
    public :
        Server(Parse par);
        void    serverSockCreate();
        void    startCommunication();
        void    createNewConnection();
        void    removeUser(int fd);
        ~Server();
};

#endif