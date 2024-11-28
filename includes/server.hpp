#ifndef SERVER_HPP
# define SERVER_HPP

#include "client.hpp"
# include "irc.hpp"

#define MAXCLIENT 100

using namespace std;
class Client;

class Server
{
    private :
        static bool     Signal;
        int             ServerPort;
        string          ServerPassword;
        string          ServerIP;
        int             socketfd;
        int             epollFd;
        vector<Client>  usersList;
        vector<Channel> channels;
    
    public :
        std::string    getServerPassword( void ){return (ServerPassword);};
        std::vector<Client> getUsersList( void ){return (usersList);};
        std::vector<Channel> getChannels( void ){return (channels);};
        std::string    getServerIP( void ){return (ServerIP);};

        
        Server(Parse par);
        void    serverSockCreate();
        void    startCommunication();
        void    createNewConnection();
        void    removeUser(int fd);
        void    closeAllFds( void );
        bool    getSignal( void ){return (Signal);};
        static void    setSignal(bool s){Signal = s;};
        static void    signal_handler(int);

        void    SendMsg(int socketFd, std::string msg);
        std::string RecvMsg(int socketFd);
        Client    UserAuth(int userFd);
        void NumericReplies(Client client, bool flag);
       ~Server();
};



#endif