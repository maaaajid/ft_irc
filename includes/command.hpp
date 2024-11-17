#ifndef COMMAND_HPP
# define COMMAND_HPP

# include "irc.hpp"

class Command {
public:
    static void handleCommand(const std::string &command, Client &client, std::vector<Channel> &channels);
};

#endif