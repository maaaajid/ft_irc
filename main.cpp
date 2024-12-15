#include "includes/server.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        cerr << "Usage : ./ircserv <port> <password>" << endl;
        return (1);
    }
    else
    {
        try
        {
            Server *multiplexing = new Server(parse(av));
            (void)multiplexing;
        }
        catch(const std::exception& e)
        {
            // debug.logDebug(e.what());
        }
    }
}
