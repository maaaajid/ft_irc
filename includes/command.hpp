#ifndef COMMAND_HPP
# define COMMAND_HPP

# include "irc.hpp"
class Client;
class Server;
// # include "client.hpp"

class Command {
public:
    void handleCommand(std::vector<std::string> &commands, Client &client,  Server &server, epoll_event *events);
    static std::vector<std::string> getTheCommand(std::string &command);
    void   setModeHandler(std::vector<std::string> &commands, Client &client, Server &server);
};

#endif