#ifndef REPLIES_HPP
#define REPLIES_HPP

# include "irc.hpp"

class Replies
{
    public:
        static void RPL_WELCOME(Client &client, Server &server);
        static void RPL_YOURHOST(Client &client, Server &server);
        static void RPL_CREATED(Client &client, Server &server);
        static void RPL_MYINFO(Client &client, Server &server);
        static void welcomeRpl(Client &client, Server &server);
        void sendBanner(Client &client, Server &server);
        static void NRP_INCORRECTPASS(Client &client);
        static void NRP_UNKOWNCMD(Client &client, std::string cmd);
        static void NRP_NICKINUSE(Client &client);
};

#endif