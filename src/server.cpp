#include "../includes/server.hpp"
#include "../includes/client.hpp"
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>

Server::Server(Parse par)
{
    //setting port and password after parse;
    this->ServerIP = "127.0.0.1";
    this->ServerPort = par.port;
    this->ServerPassword = par.password;

    //creating an epoll instence in the kernel that will track all fds that we'll create later;
    if ((this->epollFd = epoll_create(1)) < 0)
        throw runtime_error("Erorr: epoll_create()");

    // now we'll create a server socket;
    this->serverSockCreate();

    // after we created a server socket, now we will start;
    this->startCommunication();
}

Server::~Server(){}

void    Server::serverSockCreate()
{
    // creating a server socket that will tell as if there is any new client;
    if ((this->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket() error"), throw runtime_error("error socket()");


    // add an option to make socket able to reuse same address and port,
    // we don't need to wait WAIT_TIME to use same ip and port;
    int optval = 1;
    if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        perror("setsockopt() error"), throw runtime_error("error setsockopt()");


    // fill struct sockaddr_in whit socket ip and port we will use it after;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->ServerPort);
    if (inet_pton(AF_INET, this->ServerIP.c_str(), &addr.sin_addr.s_addr) < 0)
        perror("inet_pton() error"), throw runtime_error("error inet_pton()");


    // here we bind socket file discreptor whit the IP and port we want;
    if (bind(this->socketfd, &(sockaddr &)addr, sizeof(addr)) < 0)
        perror("bind() error"), throw runtime_error("error bind()");


    // here the program will stop until a client reach socket; 
    if (listen(this->socketfd, SOMAXCONN) < 0)
        perror("listen() error"), throw runtime_error("error listen()");


    // here we add the server file discreptor to epoll inctense to tell as if there is any new client;
    epoll_event connection;
    connection.events = EPOLLIN;
    connection.data.fd = this->socketfd;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->socketfd, &connection) < 0)
        perror("epoll_ctl() error"), throw runtime_error("error epoll_ctl()");
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
        perror("accept() error"), throw runtime_error("error accept()");
    // if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    //     perror("fcntl() error"), throw runtime_error("error fcntl()");

    // we add the new client to epoll instece to check it if it has any event;
    Client clientData;
    memset(&newClient, 0, sizeof(newClient));
    newClient.data.fd = fd;
    newClient.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &newClient) < 0)
        perror("epoll_ctl() error"), throw runtime_error("error epoll_ctl()");
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

    while (1337)
    {
        //epoll_wait() is the function that will tell as how many event hapening; 
        epollCounter = epoll_wait(this->epollFd, events, 1024, -1);
        if (epollCounter < 0)
            perror("epoll_wait() error"), throw runtime_error("error epoll_wait()");

        
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
                cout << "in fd: "<< events[x].data.fd << " " << buffer;
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
                perror("epoll_ctl() error"), throw runtime_error("error epoll_ctl()");
            cout << fd << " deleted" << endl;
            break;
        }
        it++;
    }
}