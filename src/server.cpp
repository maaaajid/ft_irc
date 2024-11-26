
#include "../includes/irc.hpp"

bool Server::Signal = false;

Server::Server(Parse par)
{
    this->ServerIP = "127.0.0.1";
    this->ServerPort = par.port;
    this->ServerPassword = par.password;
    if ((this->epollFd = epoll_create(1)) < 0)
        logger.logError("Error creating epoll instance: " + string(strerror(errno))), throw runtime_error("Error creating epoll instance");
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
        logger.logError("Error creating socket: " + string(strerror(errno))), throw runtime_error("Error creating socket");
    int optval = 1;
    if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        logger.logError("Error setting socket option: " + string(strerror(errno))), throw runtime_error("Error setting socket option");
    if (fcntl(this->socketfd, F_SETFL, O_NONBLOCK) < 0)
        perror("fcntl() error"), throw runtime_error("error fcntl()");
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->ServerPort);
    if (inet_pton(AF_INET, this->ServerIP.c_str(), &addr.sin_addr.s_addr) < 0)
        logger.logError("Error converting IP address: " + string(strerror(errno))), throw runtime_error("Error converting IP address");
    if (bind(this->socketfd, &(sockaddr &)addr, sizeof(addr)) < 0)
        logger.logError("Error binding socket: " + string(strerror(errno))), throw runtime_error("Error binding socket"); 
    if (listen(this->socketfd, SOMAXCONN) < 0)
        logger.logError("Error listening on socket: " + string(strerror(errno))), throw runtime_error("Error listening on socket");
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

    memset(&addr, 0, sizeof(addr));
    socklen = sizeof(addr);
    if ((fd = accept(this->socketfd, &(sockaddr &)addr, &socklen)) < 0)
        logger.logError("Error accepting new connection: " + string(strerror(errno))), throw runtime_error("Error accepting new connection");
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
        perror("fcntl() error"), throw runtime_error("error fcntl()");
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
    epoll_event     events[1024];
    signal(SIGINT, Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler);

    while (!this->Signal)
    {
        epollCounter = epoll_wait(this->epollFd, events, 1024, -1);
        if (epollCounter < 0)
                logger.logError("Error waiting for events: " + string(strerror(errno))), throw runtime_error("Error waiting for events");
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
                    //printng the commands
                    // std::cout << "buffer: " << buffer << std::endl;
                    // for (size_t i= 0; i < cmds.size(); i++)
                    //     std::cout << "cmds: " << cmds[i] << std::endl;
                    Command::handleCommand(cmds, *it, *this);
                    std::cout << "is user authed= " << it->getAuth() << std::endl;
                    std::cout << "Nick rec = " << it->getnickName() << std::endl;
                    std::cout << "USer rec = " << it->getuserName() << std::endl;    

                }
            }
        }
    }
}

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
            logger.logInfo("Deleted user: '" + it->getuserName() + "' fd: " + toString(it->getC_fd()));
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
        logger.logWarning("closing fd: '" + toString(it->getC_fd()));
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

std::string Server::RecvMsg(int socketFd)
{
    char buffer[MAX_READ_ONCE];
    int recvNum;

    recvNum = recv(socketFd, buffer, MAX_READ_ONCE, 0);
    //check this logger is Warning or Error.
    if (recvNum == 0)
        logger.logWarning("closed Connection");
    return string(buffer, recvNum);
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