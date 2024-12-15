#include "../includes/irc.hpp"

//:server 001 <nick> :Welcome to the <network> Network, <nick>[!<user>@<host>]
void Replies::RPL_WELCOME(Client &client, Server &server)
{
    (void)server;
    client.sendMessage(":server 001 " +client.getnickName() + " : Welcome The " + server.getServerName() + ", " + client.getnickName());
}
// :server 002 <nick> :Your host is <servername>, running version <version>
void Replies::RPL_YOURHOST(Client &client, Server &server)
{
    client.sendMessage(":server 002 " + client.getnickName()+ " : Your host is " + server.getServerName() + ", running version V1");
}
//:server 003 <nick> :This server was created <datetime>

void Replies::RPL_CREATED(Client &client, Server &server)
{
    (void)server;
    std::string datetime = logger.getTime();
    client.sendMessage(":server 003 " + client.getnickName()+ " :This server was created" + datetime); 
}
// :server 004 <nick> <servername> <version> <available umodes> <available cmodes> [<cmodes with param>]
void Replies::RPL_MYINFO(Client &client, Server &server)
{
    (void)server;
    client.sendMessage(":server 004 " + client.getnickName()+ " " + server.getServerName() + ", Avilibale cmd: []");
}

// void Replies::sendBanner(Client &client, Server &server) 
// {
//     (void)server;
//     std::stringstream banner;
//     banner << "*************************************************************************\n"
//            << "*                                                                       *\n"
//            << "*            WELCOME TO " << "DoubleM-B" << " IRC NETWORK!                     *\n"
//            << "*                                                                       *\n"
//            << "*   Hi " << client.getnickName() << ", we're glad to have you here!                *\n"
//            << "*   Our server is running version " << "version lawala" << ".                     *\n"
//            << "*   Created on: " << "nehar l7ed mea juj" << ".                                *\n"
//            << "*                                                                       *\n"
//            << "*      Remember to follow the rules and have fun chatting!             *\n"
//            << "*                                                                       *\n"
//            << "*             Type /help to get started or join a channel!             *\n"
//            << "*                                                                       *\n"
//            << "*************************************************************************";

//     client.sendMessage(":" + banner.str());
// }

void Replies::welcomeRpl(Client &client, Server &server)
{
    RPL_WELCOME(client, server);
    RPL_YOURHOST(client, server);
    RPL_CREATED(client, server);
    RPL_MYINFO(client, server);
    
}