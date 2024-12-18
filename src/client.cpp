#include "../includes/client.hpp"

Client::Client(){
    c_fd            = -1;
    nickname        = "";
    username        = "";
    c_ip            = "";
    c_state         = 0;
    auth            = false;
    passValid       = false;
    nickValid       = false;
    userValid       = false;
    invisible       = false;
    auth            = false;
    passValid       = false;
    nickValid       = false;
    userValid       = false;
    invisible       = false;
    cmdComplete     = false;
}

Client::~Client(){}

void    Client::setC_fd(int fd) {this->c_fd = fd; }

void    Client::setNickname(std::string nickName) {this->nickname = nickName; }

void    Client::setUsername(std::string userName) {this->username = userName; }

void    Client::setC_ip(std::string c_Ip) {this->c_ip = c_Ip; }

void    Client::setC_state(int c_State) {this->c_state = c_State; }

int     Client::getC_fd(void) const {return(this->c_fd); }

void Client::appendToBuffer(const std::string& data)
{
    buffer += data;
    if (buffer.find('\n') != std::string::npos)
        cmdComplete = true;
    else
        cmdComplete = false;
}

std::string Client::getBuffer() const { return buffer; }

bool Client::commandComplete() const { return cmdComplete; }

void Client::clearBuffer()
{
    buffer.clear();
    cmdComplete = false;
}

std::string  Client::getnickName(void)
{
    if (nickname.empty())
    {
        logger.logError("Nickname is empty");
        return "";
    }
    return (this->nickname);
}

std::string  Client::getuserName(void) {return (this->username); }

std::string  Client::getC_ip(void) {return (this->c_ip); }

int     Client::getC_state(void) {return (this->c_state); }

void    Client::setAuth(bool auth) { this->auth = auth; }

bool    Client::getAuth(void) { return (this->auth); }

bool    Client::getPassValid(void) {return (this->passValid);}

void    Client::setPassValid(bool valid) { this->passValid = valid;}


//1
void        Client::passHandler(std::vector<std::string> &commands, Server &server, epoll_event *events)
{
    if (this->passValid)
    {
        logger.logError("PASS command already sent.");
        return;
    }
    if (commands.size() < 2)
    {
        logger.logError("Invalid PASS command.");
        return;
    }

    std::string password = commands[1];
    if (password == server.getServerPassword())
    {
        logger.logInfo("Password accepted.");   
        sendMessage("001 : Password accepted, continue with NICK.");
        setPassValid(true);
    }
    else
    {
        setPassValid(false);
        sendMessage("421 : Password rejected.");
        server.removeUser(this->c_fd, events);
        throw std::runtime_error("anqi");
        return;
    }
}

//1
void        Client::nickHandler(std::string &command, Server &server)
{
    if (!this->passValid)
    {
        sendMessage("Password not set!");
        return ;
    }
    if (!isValidNick(command))
    {
        sendMessage("Invalid nickname.");
        return;
    }
    std::vector<Client*> usersList = server.getUsersList();
    for (std::vector<Client*>::iterator it = usersList.begin(); it != usersList.end(); ++it)
    {
        if ((*it)->getnickName() == command)
        {
            logger.logError("Nickname already taken.");
            return;
        }
    }
    if (command.size() > MAXNICKLEN)
    {
        logger.logError("Nickname too long.");
        return;
    }
    setNickname(command);
    sendMessage("Your nickname is : " + command);
    if (this->getuserName().empty())
        sendMessage("Set your username using /user [YOUR_USERNAME]");
    this->nickValid = true;
}

//1
void        Client::userHandler(std::string &command, Server &server)
{
    if (!this->passValid && !this->nickValid)
    {
        sendMessage("Please enter the password of the server using: /connect localhost <port> <password>");
        return ;
    }
    else if (this->passValid && !this->nickValid)
    {
        sendMessage("Please enter the nick using: /nick [YOUR_NICKNAME]");
        return ;
    }
    std::vector<Client*> usersList = server.getUsersList();
    for (std::vector<Client*>::iterator it = usersList.begin(); it != usersList.end(); ++it)
    {
        if ((*it)->getuserName() == command)
        {
            // std::cout << "cmd = " << command << " | username= " <<  it->getuserName() << std::endl;
            logger.logError("Username already taken.");
            return;
        }
    }
    if (command.empty())
    {
        logger.logError("Username can't be empty");
        return;
    }
    if (command.size() > MAXUSERNAMELEN)
    {
        logger.logError("Username too long.");
        return;
    }
    setUsername(command);
    sendMessage("Your username is : " + command + " you can now start chatting.");
    this->userValid = true;
    if (!this->auth && (this->passValid && this->nickValid && this->userValid))
        Replies::welcomeRpl(*this, server);
    this->auth = this->passValid && this->nickValid && this->userValid;
}

// void        Client::quitHandler()
// {

// }
void        Client::setInvisible(bool inv) { this->invisible = inv; }

void Client::sendMessage(const std::string &message)
{
    if (c_fd < 0)
    {
        logger.logError("Cannot send message, invalid socket.");
        return;
    }

    std::string formattedMessage = message + "\r\n";

    ssize_t bytesSent = send(c_fd, formattedMessage.c_str(), formattedMessage.size(), 0);
    if (bytesSent < 0)
        logger.logError("Error sending message to client: " + std::string(strerror(errno)));
    else if (bytesSent < static_cast<ssize_t>(formattedMessage.size()))
    {
        std::stringstream oss;
        oss << "Partial message sent to client, sent " << bytesSent << " bytes.";
        logger.logError(oss.str());
    }
    else
        logger.logInfo("Message sent to client: " + nickname + " : " + message);
}

bool    isValidNick(const std::string& nick)
{
    if (nick.empty())
        return false;
    const std::string invalidChars = " ,*?!@.";
    if (nick.find_first_of(invalidChars) != std::string::npos)
        return false;
    char firstChar = nick[0];
    const std::string invalidFirstChars = "$:#&~@%+";
    if (invalidFirstChars.find(firstChar) != std::string::npos)
        return false;
    return true;
}

void Client::changeNick(const std::string &newNick, Server &server)
{
    if (newNick.empty())
    {
        sendMessage("ERROR :Nickname cannot be empty");
        return;
    }

    std::vector<Client*> clients = server.getUsersList();
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        logger.logDebug((*it)->getnickName());
        logger.logDebug(newNick);
        if ((*it)->getnickName() == newNick)
        {
            sendMessage("ERROR :Nickname is already in use");
            return;
        }
    }
    std::string oldNick = getnickName();
    setNickname(newNick);
    
    std::string message = ":" + oldNick + " NICK :" + newNick;
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        if ((*it)->getC_fd() != getC_fd())
            (*it)->sendMessage(message);
    
    sendMessage("NICK changed to " + newNick);
}
