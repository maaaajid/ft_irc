#include "../includes/server.hpp"

Parse    parse(char **av)
{
    Parse parse;
    int x = -1;
    while (av[2][++x])
    {
        if (!isalnum(av[2][x]))
            cout << "Something wrong with the password" << endl, exit (1);
        parse.password = av[2];
    }
    x = -1;
    while (av[1][++x])
    {
        if (!isdigit(av[1][x]))
            cout << "Non numeric character in the port number" << endl, exit (1);
    }
    if (atoi(av[1]) < 1024 && atoi(av[1]) > 65535)
        cout << "Port number out of range" << endl, exit (1);
    parse.port = atoi(av[1]);
    return (parse);
}