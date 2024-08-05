#ifndef LOGGER_HPP
# define LOGGER_HPP

# include "irc.hpp"

//Reset
#define RST "\e[0m"

//Regular text
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

class Logger
{
    private :
        std::string getTime();

    public :
        Logger();
        ~Logger();
        void    logInfo(std::string msg);
        void    logDebug(std::string msg);
        void    logWarning(std::string msg);
        void    logError(std::string msg);
};

#endif