#include "../includes/irc.hpp"

void Replies::RPL_WELCOME(Client &client, Server &server)
{
    (void)server;
    client.sendMessage("001 " + client.getnickName() + " :Welcome to the Internet Relay Network " + client.getnickName() + "!" + client.getuserName() + "@" + client.getC_ip());
}

void Replies::RPL_YOURHOST(Client &client, Server &server)
{
    client.sendMessage("002 " + client.getnickName() + " :Your host is " + server.getServerIP() + ", running version 1.0");
}

void Replies::RPL_CREATED(Client &client, Server &server)
{
    (void)server;
    client.sendMessage("003 " + client.getnickName() + " :This server was created " + "2021-07-01"); 
}

void Replies::RPL_MYINFO(Client &client, Server &server)
{
    (void)server;
    client.sendMessage("004 " + client.getnickName() + " :localhost 1.0 ao mt");
}

void Replies::sendBanner(Client &client, Server &server) 
{
    (void)server;
    std::stringstream banner;
    banner << "*************************************************************************\n"
           << "*                                                                       *\n"
           << "*            WELCOME TO " << "DoubleM-B" << " IRC NETWORK!                     *\n"
           << "*                                                                       *\n"
           << "*   Hi " << client.getnickName() << ", we're glad to have you here!                *\n"
           << "*   Our server is running version " << "version lawala" << ".                     *\n"
           << "*   Created on: " << "nehar l7ed mea juj" << ".                                *\n"
           << "*                                                                       *\n"
           << "*      Remember to follow the rules and have fun chatting!             *\n"
           << "*                                                                       *\n"
           << "*             Type /help to get started or join a channel!             *\n"
           << "*                                                                       *\n"
           << "*************************************************************************";

    client.sendMessage(":" + banner.str());
}

void Replies::welcomeRpl(Client &client, Server &server)
{
    RPL_WELCOME(client, server);
    RPL_YOURHOST(client, server);
    RPL_CREATED(client, server);
    RPL_MYINFO(client, server);
}