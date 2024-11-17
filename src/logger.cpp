#include "../includes/irc.hpp"

Logger::Logger(){}

Logger::~Logger(){}

std::string Logger::getTime()
{
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::stringstream ss;
        ss << "[" << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":"
           << std::setfill('0') << std::setw(2) << ltm->tm_min << ":"
           << std::setfill('0') << std::setw(2) << ltm->tm_sec << "] ";
        return ss.str();
}

void    Logger::logInfo(std::string msg)
{
    std::cout << MAG << getTime() << BLU << "[INFO]" << RST << " " << msg << std::endl;
}

void    Logger::logDebug(std::string msg)
{
    std::cout << MAG << getTime() << GRN << "[DEBUG]" << RST << " " << msg << std::endl;
}

void    Logger::logError(std::string msg)
{
    std::cerr << MAG << getTime() << RED << "[ERROR]" << RST << " " << msg << std::endl;
}

void    Logger::logWarning(std::string msg)
{
    std::cout << MAG << getTime() << YEL << "[WARNING]" << RST << " " << msg << std::endl;
}

Logger logger;