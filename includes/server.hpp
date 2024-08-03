#ifndef SERVER_HPP
# define SERVER_HPP

# include "irc.hpp"

using namespace std;
class Client;

class Server
{
    private :
        int             ServerPort;
        string          ServerPassword;
        string          ServerIP;
        int             socketfd;
        int             epollFd;
        vector<Client>  usersList;
    
    public :
        Server(Parse par);
        void    serverSockCreate();
        void    startCommunication();
        void    createNewConnection();
        void    removeUser(int fd, epoll_event *events);
        ~Server();
};

#endif