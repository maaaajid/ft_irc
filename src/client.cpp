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

void    Client::setPassValid(bool valid) { this->passValid = valid, this->c_state = AUTHENTICATED; }

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