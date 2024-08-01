#ifndef UTILS_HPP
# define UTILS_HPP

# include "irc.hpp"

struct Parse
{
    int     port;
    std::string  password;
    std::string  IPaddr;
};

Parse    parse(char **av);

#endif