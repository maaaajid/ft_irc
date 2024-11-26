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
        // static void RPL_LUSERCLIENT(Client &client);



        // static void RPL_ISUPPORT(Client &client);
        // static void RPL_LUSERCLIENT(Client &client);
        // static void RPL_LUSEROP(Client &client);
        // static void RPL_LUSERUNKNOWN(Client &client);
        // static void RPL_LUSERCHANNELS(Client &client);
        // static void RPL_LUSERME(Client &client);
        // static void RPL_MOTDSTART(Client &client);
        // static void RPL_MOTD(Client &client);
        // static void RPL_ENDOFMOTD(Client &client);
        // static void ERR_NOSUCHNICK(Client &client);
        // static void ERR_NOSUCHSERVER(Client &client);
        // static void ERR_NOSUCHCHANNEL(Client &client);
        // static void ERR_CANNOTSENDTOCHAN(Client &client);   
};

#endif