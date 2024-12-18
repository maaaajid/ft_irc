
#include "../includes/irc.hpp"

bool Server::Signal = false;

Server::Server(Parse par)
{
    this->ServerIP = "127.0.0.1";
    this->ServerPort = par.port;
    this->ServerName = "Double_MB";
    this->ServerPassword = par.password;
    usersList.reserve(MAXCLIENT);
    if ((this->epollFd = epoll_create(1)) < 0)
        logger.logError("Error creating epoll instance: " + std::string(strerror(errno))), throw runtime_error("Error creating epoll instance");
    this->serverSockCreate();
    logger.logInfo("Server Started !");
    logger.logInfo("Server Ip: " + this->ServerIP + " , Port: "+ toString(this->ServerPort));
    logger.logInfo("Waiting connections...");
    this->startCommunication();
    this->closeAllFds();
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        for (size_t i = 0; i < usersList.size(); ++i)
            delete usersList[i];
        usersList.clear();
        for (size_t i = 0; i < other.usersList.size(); ++i)
            this->usersList.push_back(new Client(*other.usersList[i]));
    }
    return *this;
}

Server::~Server()
{
    for (std::vector<Client *>::iterator client; client != usersList.end(); ++client)
        delete *client;
}

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
    for (std::vector<Client*>::iterator it = usersList.begin(); it != usersList.end(); ++it)
    {
        if ((*it)->getC_fd() == fd)
        {
            logger.logWarning("Client is already connected: " + (*it)->getnickName());
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
    Client *clientData = new Client;
    clientData->setC_fd(fd);
    clientData->setC_ip(inet_ntoa(addr.sin_addr));
    usersList.push_back(clientData);
}

void    Server::startCommunication()
{
    int             epollCounter;
    int             x = 0;
    int             i = 0;
    epoll_event     events[MAX_READ_ONCE];
    signal(SIGINT, Server::signal_handler);
    signal(SIGQUIT, Server::signal_handler);
    Command command;
    Bot     bot;

    while (!this->Signal)
    {
        epollCounter = epoll_wait(this->epollFd, events, MAX_READ_ONCE, -1);
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
                std::vector<Client*> usersCopy = usersList;
                for (std::vector<Client*>::iterator it = usersCopy.begin(); it != usersCopy.end(); ++it)
                {
                    if ((*it)->getC_fd() == events[x].data.fd)
                    {
                        try
                        {
                            std::string buffer = RecvMsg((*it)->getC_fd());
                            (*it)->appendToBuffer(buffer);

                            if ((*it)->commandComplete())
                            {
                                std::string completeCommand = (*it)->getBuffer();
                                std::vector<std::string> cmds = Command::getTheCommand(completeCommand);

                                std::vector<Client*>::iterator originalIt = 
                                    std::find(usersList.begin(), usersList.end(), *it);
                                if (originalIt != usersList.end())
                                {
                                    command.handleCommand(cmds, *(*originalIt), *this, events);
                                    if (std::find(usersList.begin(), usersList.end(), *it) != usersList.end())
                                    {
                                        if ((i++ % 10) == 0)
                                            (*originalIt)->sendMessage("[MOTIVATIONAL QUOTE] " + bot.generateQuote());
                                        (*originalIt)->clearBuffer();
                                    }
                                }
                            }
                        }
                        catch (std::exception& e)
                        {
                            logger.logError("Error processing command: " + std::string(e.what()));
                            break;
                        }
                    }
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
    Client* clientToRemove = NULL;
    for (std::vector<Client*>::iterator it = usersList.begin(); it != usersList.end(); ++it)
    {
        if ((*it)->getC_fd() == fd)
        {
            clientToRemove = *it;
            break;
        }
    }
    if (clientToRemove)
    {
        if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, events) == -1)
        {
            logger.logError("Failed to remove client from epoll: " + std::string(strerror(errno)));
            return;
        }
        close(fd);
        usersList.erase(std::remove(usersList.begin(), usersList.end(), clientToRemove), usersList.end());
        logger.logInfo("User " + clientToRemove->getuserName() + " has been removed.");
        delete clientToRemove;
    }
    else
        logger.logWarning("Client with fd " + toString(fd) + " not found.");
}


void    Server::closeAllFds()
{
    vector<Client*>::iterator it = usersList.begin();
    while (it != usersList.end())
    {
        logger.logWarning("closing fd: '" + toString((*it)->getC_fd()));
        close((*it)->getC_fd());
        it++;
    }
    close(this->socketfd);
    close(this->epollFd);
}

void    Server::signal_handler(int) { Server::setSignal(true); }

std::string Server::RecvMsg(int socketFd)
{
    if (socketFd <= 0)
    {
        logger.logWarning("Invalid socket file descriptor.");
        throw std::runtime_error("Fd is invalid.");
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
    for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
        if ((*it)->getName() == name)
            return &(*(*it));
    return NULL;
}

void Server::addChannel(Channel *newChannel)
{
    channels.push_back(newChannel);
}

void Server::removeChannel(const std::string& channelName)
{
    for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if ((*it)->getName() == channelName)
        {
            channels.erase(it);
            break;
        }
    }
}