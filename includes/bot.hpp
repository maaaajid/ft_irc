#ifndef BOT_HPP
# define BOT_HPP

# include "irc.hpp"

class Bot
{
    public:
        Bot();
        ~Bot();
        std::string generateQuote();
};

#endif