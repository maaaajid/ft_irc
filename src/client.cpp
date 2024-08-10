#include "../includes/client.hpp"

Client::Client(){}

Client::~Client(){}

void    Client::setC_fd(int fd)
{
    this->c_fd = fd;
}

void    Client::setNickname(string nickName)
{
    this->nickname = nickName;
}

void    Client::setUsername(string userName)
{
    this->username = userName;
}

void    Client::setC_ip(string c_Ip)
{
    this->c_ip = c_Ip;
}

void    Client::setC_state(int c_State)
{
    this->c_state = c_State;
}

int Client::getC_fd(void) const
{
    return(this->c_fd);
}

string  Client::getnickName(void)
{
    return (this->nickname);
}

string  Client::getuserName(void)
{
    return (this->username);
}

string  Client::getC_ip(void)
{
    return (this->c_ip);
}

int  Client::getC_state(void)
{
    return (this->c_state);
}

void    Client::setAuth(bool auth)
{
    this->auth = auth;
}

bool    Client::getAuth(void)
{
    return (this->auth);
}