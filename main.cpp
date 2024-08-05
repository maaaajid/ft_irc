#include "includes/server.hpp"

int main(int ac, char **av)
{
    Logger debug;

    if (ac != 3)
    {
        cerr << "Usage : ./ircserv <port> <password>" << endl;
        return (1);
    }
    else
    {
        try
        {
            Server multiplexing(parse(av));
        }
        catch(const std::exception& e)
        {
<<<<<<< HEAD
            std::cerr << e.what() << std::endl;
=======
            // debug.logDebug(e.what());
>>>>>>> brahim
        }
    }
}
