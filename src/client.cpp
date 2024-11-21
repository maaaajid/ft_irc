#include "../includes/client.hpp"

Client::Client(){}

Client::~Client(){}

void    Client::setC_fd(int fd) {this->c_fd = fd; }

void    Client::setNickname(string nickName) {this->nickname = nickName; }

void    Client::setUsername(string userName) {this->username = userName; }

void    Client::setC_ip(string c_Ip) {this->c_ip = c_Ip; }

void    Client::setC_state(int c_State) {this->c_state = c_State; }

int Client::getC_fd(void) const {return(this->c_fd); }

string  Client::getnickName(void) {return (this->nickname); }

string  Client::getuserName(void) {return (this->username); }

string  Client::getC_ip(void) {return (this->c_ip); }

int  Client::getC_state(void) {return (this->c_state); }

void    Client::setAuth(bool auth) { this->auth = auth; }

bool    Client::getAuth(void) { return (this->auth); }

bool    Client::getPassValid(void) {return (this->passValid);}

void    Client::setPassValid(bool valid) { this->passValid = valid;}


//0 baqi khassni ntesti some conditions.
void        Client::passHandler(std::vector<std::string> &commands, Server &server)
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
    // std::cout << "password :  " << password << std::endl;
    // std::cout << "server password :  " << server.getServerPassword() << std::endl;
    if (password == server.getServerPassword())
    {
        sendMessage("Password accepted.");
        setPassValid(true);
    }
    else
    {
        sendMessage("Password rejected.");
        setPassValid(false);
    }
    this->passValid = true;
    this->auth = this->passValid && this->nickValid && this->userValid;


}

//0 still need to check if the nick is already taken.
// aykhassni ndwez instance mn server bach nchecki.
// aykhassni nzid n7et limit f nicknames.
void        Client::nickHandler(std::vector<std::string> &commands)
{
    if (this->nickValid)
    {
        logger.logError("NICK command already sent.");
        return;
    }
    if (commands.size() < 2)
    {
        logger.logError("Invalid NICK command.");
        return;
    }

    std::string nick = commands[1];
    if (!isValidNick(nick))
    {
        sendMessage("Invalid nickname.");
        return;
    }
    setNickname(nick);
    this->nickValid = true;
    if (!this->auth && (this->passValid && this->nickValid && this->userValid))
	    std::cout << "Welcome to the IRC server!" << std::endl;
    this->auth = this->passValid && this->nickValid && this->userValid;
}

//0 still need to check if the username is already taken.
// aykhassni ndwez instance mn server bach nchecki.
// aykhassni nzid n7et limit f usernames.
void        Client::userHandler(std::vector<std::string> &commands)
{
    if (this->userValid)
    {
        logger.logError("USER command already sent.");
        return;
    }
    if (commands.size() < 5)
    {
        logger.logError("Invalid USER command.");
        return;
    }

    std::string username = commands[1];
    setUsername(username);
    this->userValid = true;
    if (!this->auth && (this->passValid && this->nickValid && this->userValid))
        std::cout << "Welcome to the IRC server!" << std::endl;
    this->auth = this->passValid && this->nickValid && this->userValid;
}

// void        Client::quitHandler(std::vector<std::string> &commands);

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
        std::ostringstream oss;
        oss << "Partial message sent to client, sent " << bytesSent << " bytes.";
        logger.logError(oss.str());
    }
    else
        logger.logInfo("Message sent to client: " + message);
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