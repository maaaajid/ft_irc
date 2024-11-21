#ifndef COMMAND_HPP
# define COMMAND_HPP

# include "irc.hpp"
class Client;
class Server;
// # include "client.hpp"

class Command {
public:
    static void handleCommand(std::vector<std::string> &commands, Client &client,  Server &server);
    static std::vector<std::string> getTheCommand(std::string &command);
    // DRY hhh addi hadi abro bach dir cmd f vector hssen men string
};

#endif