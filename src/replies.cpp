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
    client.sendMessage(":server 004 " + client.getnickName()+ " " + server.getServerName() + ", Avilibale cmds: [PASS, NICK, USER, JOIN, TOPIC, MODE, KICK, INVITE, PRIVMSG, PART, QUIT]");
}

void Replies::welcomeRpl(Client &client, Server &server)
{
    RPL_WELCOME(client, server);
    RPL_YOURHOST(client, server);
    RPL_CREATED(client, server);
    RPL_MYINFO(client, server);
    
}

//464 <username> :Password incorrect - Indicates incorrect password.
void Replies::NRP_INCORRECTPASS(Client &client)
{
    client.sendMessage(":server 464 " + client.getnickName()+ " :Password incorrect");
}
// 501 <username> :Unknown command - Indicates unknown command.
void Replies::NRP_UNKOWNCMD(Client &client, std::string cmd)
{
    client.sendMessage(":server 501 " + client.getnickName()+ " :Unknown command " + cmd);
}

//433 <username> <nickname> :Nickname is already in use. - Indicates nickname is already in use.
void Replies::NRP_NICKINUSE(Client &client)
{
    client.sendMessage(":server 433 " + client.getnickName()+ " :Nickname is already in use");
}