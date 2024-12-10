
#include "../includes/irc.hpp"

bool Server::Signal = false;

Server::Server(Parse par)
{
    this->ServerIP = "127.0.0.1";
    this->ServerPort = par.port;
    this->ServerPassword = par.password;
    if ((this->epollFd = epoll_create(1)) < 0)
        logger.logError("Error creating epoll instance: " + std::string(strerror(errno))), throw runtime_error("Error creating epoll instance");
    this->serverSockCreate();
    logger.logInfo("Server Started !");
    logger.logInfo("Server Ip: " + this->ServerIP + " , Port: "+ toString(this->ServerPort));
    logger.logInfo("Waiting connections...");
    this->startCommunication();
    this->closeAllFds();
}

Server::~Server(){}

void    Server::serverSockCreate()
{
    if ((this->socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        logger.logError("Error creating socket: " + std::string(strerror(errno))), throw runtime_error("Error creating socket");
    int optval = 1;
    if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        logger.logError("Error setting socket option: " + std::string(strerror(errno))), throw runtime_error("Error setting socket option");
    if (fcntl(this->socketfd, F_SETFL, O_NONBLOCK) < 0)
        perror("fcntl() error"), throw runtime_error("error fcntl()");
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->ServerPort);
    if (inet_pton(AF_INET, this->ServerIP.c_str(), &addr.sin_addr.s_addr) < 0)
        logger.logError("Error converting IP address: " + std::string(strerror(errno))), throw runtime_error("Error converting IP address");
    if (bind(this->socketfd, &(sockaddr &)addr, sizeof(addr)) < 0)
        logger.logError("Error binding socket: " + std::string(strerror(errno))), throw runtime_error("Error binding socket"); 
    if (listen(this->socketfd, SOMAXCONN) < 0)
        logger.logError("Error listening on socket: " + std::string(strerror(errno))), throw runtime_error("Error listening on socket");
    epoll_event connection;
    connection.events = EPOLLIN;
    connection.data.fd = this->socketfd;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->socketfd, &connection) < 0)
        logger.logError("Error adding socket to epoll: " + std::string(strerror(errno))), throw runtime_error("Error adding socket to epoll");
}

void    Server::createNewConnection()
{
    int fd;
    socklen_t socklen;
    sockaddr_in addr;
    epoll_event newClient;

    memset(&addr, 0, sizeof(addr));
    socklen = sizeof(addr);
    if ((fd = accept(this->socketfd, (sockaddr *)&addr, &socklen)) < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        logger.logError("Error accepting new connection: " + std::string(strerror(errno)));
        return;
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        logger.logError("fcntl() error: " + std::string(strerror(errno)));
        close(fd);
        return;
    }
    for (std::vector<Client>::iterator it = usersList.begin(); it != usersList.end(); ++it)
    {
        if (it->getC_fd() == fd)
        {
            logger.logWarning("Client is already connected: " + it->getnickName());
            close(fd);
            return;
        }
    }

    memset(&newClient, 0, sizeof(newClient));
    newClient.data.fd = fd;
    newClient.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &newClient) < 0)
    {
        logger.logError("Error adding new connection to epoll: " + std::string(strerror(errno)));
        close(fd);
        return;
    }
    Client clientData;
    clientData.setC_fd(fd);
    clientData.setC_ip(inet_ntoa(addr.sin_addr));
    usersList.push_back(clientData);
}

void    Server::startCommunication()
{
    int             epollCounter;
    int             x = 0;
    epoll_event     events[1024];
    signal(SIGINT, Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler);
    Command command;

    while (!this->Signal)
    {
        epollCounter = epoll_wait(this->epollFd, events, 1024, -1);
        if (epollCounter < 0)
        {
            logger.logError("Error waiting for events: " + std::string(strerror(errno))), throw runtime_error("Error waiting for events");
            continue;
        }
        x = -1;
        while (++x < epollCounter)
        {
            if (events[x].data.fd == this->socketfd)
                this->createNewConnection();
            if (events[x].events & EPOLLERR || events[x].events & EPOLLHUP || events[x].events & EPOLLRDHUP)
            {
                this->removeUser(events[x].data.fd, events);
                continue;
            }
            if (events[x].events & EPOLLIN && events[x].data.fd != socketfd)
            {
                for (std::vector<Client>::iterator it = this->usersList.begin(); it != this->usersList.end(); ++it)
                {
                    std::string buffer = RecvMsg(it->getC_fd());
                    std::vector<std::string> cmds = Command::getTheCommand(buffer);
                    command.handleCommand(cmds, *it, *this, events);
                    if (usersList.empty())
                        break;
                }
            }
        }
    }
}

void Server::removeUser(int fd, epoll_event *events)
{
    if (fd < 0)
    {
        logger.logWarning("Invalid file descriptor!");
        return;
    }
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, events) == -1)
    {
        logger.logError("Failed to remove client from epoll: " + std::string(strerror(errno)));
        return;
    }
    close(fd);

    for (std::vector<Client>::iterator it = usersList.begin(); it != usersList.end(); ++it)
    {
        if (it->getC_fd() == fd)
        {
            logger.logInfo("User " + it->getuserName() + " has been removed.");
            usersList.erase(it);
            break;
        }
    }
}


void    Server::closeAllFds()
{
    vector<Client>::iterator it = usersList.begin();
    while (it != usersList.end())
    {
        logger.logWarning("closing fd: '" + toString(it->getC_fd()));
        close(it->getC_fd());
        it++;
    }
    close(this->socketfd);
    close(this->epollFd);
}

void    Server::signal_handler(int) { Server::setSignal(true); }

std::string Server::RecvMsg(int socketFd)
{
    if (socketFd < 0)
    {
        logger.logWarning("Invalid socket file descriptor.");
        return std::string();
    }

    char buffer[MAX_READ_ONCE];
    int recvNum;

    recvNum = recv(socketFd, buffer, MAX_READ_ONCE, 0);
    if (recvNum == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return std::string();
        logger.logError("Error in recv(): " + std::string(strerror(errno)));
        return std::string();
    }
    if (recvNum == 0)
        logger.logWarning("closed Connection");
    return std::string(buffer, recvNum);
}

Channel* Server::getChannelByName(std::string name)
{
    for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
        if (it->getName() == name)
            return &(*it);
    return NULL;
}

void Server::addChannel(Channel newChannel)
{
    channels.push_back(newChannel);
}

// void Server::SendMsg(int socketFd, std::string msg)
// {
//     std::string newMsg = msg + "\r\n";
//     send(socketFd, newMsg.c_str(), newMsg.length(), 0);
// }

// void Server::NumericReplies(Client client, bool flag)
// {
//     if (flag)
//     {
//         SendMsg(client.getC_fd(), ":localhost 001 " + client.getnickName() + ": Welcome to the Internet Relay Network " + client.getnickName());
//         SendMsg(client.getC_fd(), ":localhost 002 " + client.getnickName() + ": Your host is localhost, running version 1.0");
//         SendMsg(client.getC_fd(), ":localhost 003 " + client.getnickName()  + ":This server is Created by Double-MB");
//         // SendMsg();
//         logger.logInfo("New user authenticated, username: " + client.getuserName() + ", fd: "+ toString(client.getC_fd()));
//     }
//     else
//     {
//        SendMsg(client.getC_fd(), ":localhost 464 " + client.getnickName() + " : Unsuccesful Auth");
//        logger.logWarning("New client failed to authenticate, fd: " + toString(client.getC_fd()));
//     }
// }