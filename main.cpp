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
            // debug.logDebug(e.what());
        }
    }
}
