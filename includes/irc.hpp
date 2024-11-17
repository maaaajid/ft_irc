#ifndef IRC_HPP
# define IRC_HPP

# include <iostream>
# include <algorithm>
# include <ctime>
# include <iomanip>
# include <sstream>
# include <map>
# include <cstdio>
# include <vector>
# include <csignal>
# include <exception>
# include <sys/socket.h>
# include <cerrno>
# include <netinet/in.h>
# include <arpa/inet.h> 
# include <cstring>
# include <string>
# include <unistd.h>
# include <sys/epoll.h>
# include <cstdlib>
# include <fcntl.h>
# include "./utils.hpp"
# include "./channel.hpp"
# include "./client.hpp"
# include "./logger.hpp"
# include "./server.hpp"
# include "./command.hpp"

using namespace std;

extern Logger logger;

#endif