#include "../includes/irc.hpp"

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

std::string toString(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

bool isValidChannelName(const std::string& name)
{
    if (name.empty() || name[0] != '#')
        return false;

    if (name.length() > 50)
        return false;

    for (std::string::size_type i = 0; i < name.length(); ++i)
    {
        char c = name[i];
        if (c < 32 || c > 126)
            return false;
        if (c == ' ' || c == ',' || c == '\x07' || c == '\r' || c == '\n')
            return false;
    }

    if (name.length() == 1)
        return false;

    return true;
}