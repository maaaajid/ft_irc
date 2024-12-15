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
        std::string          ServerPassword;
        std::string          ServerIP;
        std::string          ServerName;
        int             socketfd;
        int             epollFd;
        vector<Client*>  usersList;
        vector<Channel*> channels;
    
    public :
        Server(Parse par);
        ~Server();
        Server& operator=(const Server& other);

        std::string             getServerPassword( void ){return (ServerPassword);};
        std::vector<Client*>     getUsersList( void ){return (usersList);};
        std::vector<Channel*>    getChannels( void ){return (channels);};
        std::string             getServerIP( void ){return (ServerIP);};
        Channel*                getChannelByName(std::string name);
        std::string             getServerName() {return (ServerName);};
        void                    serverSockCreate();
        void                    startCommunication();
        void                    createNewConnection();
        void                    removeUser(int fd, epoll_event *events);
        void                    closeAllFds( void );
        bool                    getSignal( void ){return (Signal);};
        static void             setSignal(bool s){Signal = s;};
        static void             signal_handler(int);
        void                    addChannel(Channel *newChannel);
        void                    removeChannel(const std::string& channelName);

        void                    SendMsg(int socketFd, std::string msg);
        std::string             RecvMsg(int socketFd);
        Client                  UserAuth(int userFd);
        void                    NumericReplies(Client client, bool flag);
};



#endif