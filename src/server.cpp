<<<<<<< HEAD
#include "../includes/server.hpp"
<<<<<<< HEAD
#include <csignal>
=======
#include "../includes/client.hpp"
>>>>>>> brahim
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
=======
// #include "../includes/server.hpp"
// #include "../includes/logger.hpp"
// // #include "../includes/client.hpp"
// #include <netinet/in.h>
// #include <stdexcept>
// #include <sys/epoll.h>
// #include <sys/socket.h>
// #include <vector>
#include "../includes/irc.hpp"
>>>>>>> brahim

bool Server::Signal = false;

Server::Server(Parse par)
{
    //setting port and password after parse;
    this->ServerIP = "127.0.0.1";
    this->ServerPort = par.port;
    this->ServerPassword = par.password;
    //creating an epoll instence in the kernel that will track all fds that we'll create later;
    if ((this->epollFd = epoll_create(1)) < 0)
        logger.logError("Error creating epoll instance: " + string(strerror(errno))), throw runtime_error("Error creating epoll instance");
    
    // now we'll create a server socket;
    this->serverSockCreate();

    // after we created a server socket, now we will start;
    this->startCommunication();

    // this function it'll be called in case of singnels, it will close all fds; 
    this->closeAllFds();
}

Server::~Server(){}

void    Server::serverSockCreate()
{
    // creating a server socket that will tell as if there is any new client;
    if ((this->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        logger.logError("Error creating socket: " + string(strerror(errno))), throw runtime_error("Error creating socket");


    // add an option to make socket able to reuse same address and port,
    // we don't need to wait WAIT_TIME to use same ip and port;
    int optval = 1;
    if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        logger.logError("Error setting socket option: " + string(strerror(errno))), throw runtime_error("Error setting socket option");


    // fill struct sockaddr_in whit socket ip and port we will use it after;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->ServerPort);
    if (inet_pton(AF_INET, this->ServerIP.c_str(), &addr.sin_addr.s_addr) < 0)
        logger.logError("Error converting IP address: " + string(strerror(errno))), throw runtime_error("Error converting IP address");


    // here we bind socket file discreptor whit the IP and port we want;
    if (bind(this->socketfd, &(sockaddr &)addr, sizeof(addr)) < 0)
        logger.logError("Error binding socket: " + string(strerror(errno))), throw runtime_error("Error binding socket");


    // here the program will stop until a client reach socket; 
    if (listen(this->socketfd, SOMAXCONN) < 0)
        logger.logError("Error listening on socket: " + string(strerror(errno))), throw runtime_error("Error listening on socket");


    // here we add the server file discreptor to epoll inctense to tell as if there is any new client;
    epoll_event connection;
    connection.events = EPOLLIN;
    connection.data.fd = this->socketfd;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->socketfd, &connection) < 0)
        logger.logError("Error adding socket to epoll: " + string(strerror(errno))), throw runtime_error("Error adding socket to epoll");
}

void    Server::createNewConnection()
{
    int fd;
    socklen_t socklen;
    sockaddr_in addr;
    epoll_event newClient;


    // here we accept the new client and pass to the function accept struct sockaddr to fill it whit client data;
    memset(&addr, 0, sizeof(addr));
    socklen = sizeof(addr);
    if ((fd = accept(this->socketfd, &(sockaddr &)addr, &socklen)) < 0)
        logger.logError("Error accepting new connection: " + string(strerror(errno))), throw runtime_error("Error accepting new connection");
    // if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    //     perror("fcntl() error"), throw runtime_error("error fcntl()");

    // we add the new client to epoll instece to check it if it has any event;
    Client clientData;
    memset(&newClient, 0, sizeof(newClient));
    newClient.data.fd = fd;
    newClient.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &newClient) < 0)
        logger.logError("Error adding new connection: " + string(strerror(errno))), throw runtime_error("Error adding new connection");
    clientData.setC_fd(fd);
    clientData.setC_ip(inet_ntoa(addr.sin_addr));
    usersList.push_back(clientData);
}

void    Server::startCommunication()
{
    int             epollCounter;
    int             x = 0;
    char            buffer[1024];
    epoll_event     events[1024];
    signal(SIGINT, Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler);
    while (!this->Signal)
    {
        // cout << getSignal() << endl;
        //epoll_wait() is the function that will tell as how many event hapening; 
        epollCounter = epoll_wait(this->epollFd, events, 1024, -1);
        if (epollCounter < 0)
                logger.logError("Error waiting for events: " + string(strerror(errno))), throw runtime_error("Error waiting for events");

        
        //so we need to check theme all one by one;
        x = -1;
        while (++x < epollCounter)
        {
            
            // if the event happen in the server socket file discreptor that's mean is a new client;
            // so we need to add this client;
            if (events[x].data.fd == this->socketfd)
                this->createNewConnection();


            // if the events had one or more of these macros that's mean somthing wrong hapent;
            // and we need to delete this client;
            if (events[x].events & EPOLLERR || events[x].events & EPOLLHUP || events[x].events & EPOLLRDHUP)
            {
                this->removeUser(events[x].data.fd, events);
                continue;
            }


            // here if event had EPOLLIN that's mean there is some thin you need to read;
            if (events[x].events & EPOLLIN && events[x].data.fd != socketfd)
            {
                // here you reseve the msg;
                memset(buffer, 0, 1024);
                recv(events[x].data.fd, buffer, 1024, 0);
                std::stringstream ss;
                ss << "in fd: " << events[x].data.fd << " " << buffer;
                logger.logInfo(ss.str());
                //requestHandler(usersList, events[x].data.fd);
            }
        }
    }
}



// we need to test it more
void    Server::removeUser(int fd, epoll_event *events)
{
    vector<Client>::iterator it = usersList.begin();
    while (it != this->usersList.end())
    {
        if (it->getC_fd() == fd)
        {
            usersList.erase(it);
            close(fd);
            if (epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, events) > 0)
                logger.logError("Error removing socket from epoll: " + string(strerror(errno))), throw runtime_error("Error removing socket from epoll");
            cout << fd << " deleted" << endl;
            break;
        }
        it++;
    }
}

void    Server::closeAllFds()
{
    vector<Client>::iterator it = usersList.begin();
    while (it != usersList.end())
    {
        cout << "closing fd: " << it->getC_fd() << endl;
        close(it->getC_fd());
        it++;
    }
    close(this->socketfd);
    close(this->epollFd);
    
}

void    Server::signal_handler(int)
{
    Server::setSignal(true);
}