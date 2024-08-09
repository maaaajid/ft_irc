
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
    // Client clientData;
    memset(&newClient, 0, sizeof(newClient));
    newClient.data.fd = fd;
    newClient.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &newClient) < 0)
        logger.logError("Error adding new connection: " + string(strerror(errno))), throw runtime_error("Error adding new connection");
    // clientData.setC_fd(fd);
    // clientData.setC_ip(inet_ntoa(addr.sin_addr));
    // usersList.push_back(clientData);
}

void    Server::startCommunication()
{
    int             epollCounter;
    int             x = 0;
    // char            buffer[1024];
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
               Client cc =  UserAuth(events[x].data.fd);
               cout << "auth: " << cc.getAuth() << endl;
               cout << "fd: " << cc.getC_fd() << endl;
               cout << "nick: " << cc.getnickName() << endl;
               cout << "username: " << cc.getuserName() << endl;
               if (cc.getAuth())
               {
                    NumericReplies(cc, true);
                    usersList.push_back(cc);
               }
               else
                NumericReplies(cc, false); 
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

//mbouayou:

Client    Server::UserAuth(int userFd)
{
    std::string msg = RecvMsg(userFd);
    cout << msg << endl;
    sleep(4);
    std::stringstream spliter(msg);
    std::string word;
    bool checkPass, checkNick, checkUserName;
    checkPass = checkNick = checkUserName = false;

    Client newClient;
    newClient.setC_fd(userFd);
    while (spliter >> word)
    {
        if (word == "PASS")
        {
            spliter >> word;
            cout << "pass server: "  << this->ServerPassword << endl;
            cout << "pass client: "  << word << endl;
            if (word == this->ServerPassword)
            {
                checkPass = true;
                newClient.setPassValid(true);
            }
        }
        if (word == "NICK")
        {
            spliter >> word;
            checkNick = true;
            newClient.setNickname(word);
        }
        if (word == "USER")
        {
            spliter >> word;
            checkUserName = true;
            newClient.setUsername(word);
        }
    }
    if (checkPass && checkNick && checkUserName)
        newClient.setAuth(true);
    return (newClient);
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

void Server::SendMsg(int socketFd, std::string msg)
{
    std::string newMsg = msg + "\r\n";
    send(socketFd, newMsg.c_str(), newMsg.length(), 0);
}

void Server::NumericReplies(Client client, bool flag)
{
    if (flag)
    {
        SendMsg(client.getC_fd(), ":localhost 001 " + client.getnickName() + " :Welcome to the Internet Relay Network " + client.getnickName());
        SendMsg(client.getC_fd(), ":localhost 002 " + client.getnickName() + " :Your host is localhost, running version 1.0");
        SendMsg(client.getC_fd(), ":localhost 003 " + client.getnickName()  + " :This server Created by Double-MB");
        // SendMsg();
    }
    else
    {
       SendMsg(client.getC_fd(), ":localhost 464 " + client.getnickName() + " :unsscusful auth");
    }
}