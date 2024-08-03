#include "includes/server.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        cerr << "Usage : ./Irc <port> <password>" << endl;
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
            std::cerr << e.what();
        }
    }
}
