#ifndef SERVER_HPP
# define SERVER_HPP

#include "client.hpp"
# include "irc.hpp"

class Server
{
    private :
        static bool     Signal;
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
        void    removeUser(int fd, epoll_event *events);
        void    closeAllFds( void );
        bool    getSignal( void ){return (Signal);};
        static void    setSignal(bool s){Signal = s;};
        static void    signal_handler(int);
       ~Server();
};



#endif