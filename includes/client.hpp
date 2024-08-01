#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "irc.hpp"

class client
{
    private :
        int     c_socket; //client socket
        std::string  nickname;
        std::string  username;
        std::string  c_state; // client connection state

    public :
        client(){};

};

#endif