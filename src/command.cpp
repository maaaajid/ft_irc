#include "../includes/irc.hpp"

// Function object for matching channel names
struct ChannelNameMatcher {
    std::string channelName;
    ChannelNameMatcher(const std::string &name) : channelName(name) {}
    bool operator()(Channel &channel) {
        return channel.getName() == channelName;
    }
};

void Command::handleCommand(std::vector<std::string> &commands, Client &client, Server &server)
{
    if (commands.empty())
        return;

    vector<Channel>channels = server.getChannels();
    std::string cmd = commands[0];
    std::istringstream iss;
    iss.str(cmd);

    for (size_t i = 0; i < commands.size(); i++)
    {
        if (commands[i] == "PASS")
            client.passHandler(commands, server);
        else if (commands[i] == "NICK" && i + 1 < commands.size())
            client.nickHandler(commands[i + 1], server);
        else if (commands[i] == "USER" && i + 1 < commands.size())
            client.userHandler(commands[i + 1], server);
    }

    if (cmd == "JOIN")
    {
        std::string channelName;
        iss >> channelName;

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), ChannelNameMatcher(channelName));

        if (it != channels.end())
            it->join(&client);
        else
        {
            Channel newChannel(channelName);
            newChannel.join(&client);
            channels.push_back(newChannel);
        }
    }
    else if (cmd == "PART")
    {
        std::string channelName;
        iss >> channelName;

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), ChannelNameMatcher(channelName));

        if (it != channels.end())
            it->leave(&client);
    }
    else if (cmd == "MSG")
    {
        std::string channelName;
        std::string message;
        iss >> channelName;
        std::getline(iss, message);
        
        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), ChannelNameMatcher(channelName));

        if (it != channels.end())
            it->broadcastMessage(client.getnickName() + ": " + message, &client);
    }
    else if (cmd == "TOPIC")
    {
        std::string channelName;
        std::string newTopic;
        iss >> channelName;
        std::getline(iss, newTopic);

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), ChannelNameMatcher(channelName));

        if (it != channels.end())
        {
            if (it->isOperator(&client))
            {
                it->setTopic(newTopic);
                it->broadcastMessage("New topic: " + newTopic, &client);
            }
            else
                client.sendMessage("You are not an operator in this channel.");
        }
    }
    else if (cmd == "KICK")
    {
        std::string channelName;
        std::string targetNick;
        iss >> channelName >> targetNick;

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), ChannelNameMatcher(channelName));

        if (it != channels.end())
        {
            Client *target = NULL;
            std::vector<Client*> clientList = it->getClients();
            
            for (std::vector<Client*>::iterator it = clientList.begin(); it != clientList.end(); ++it)
            {
                Client *c = *it;
                if (c->getnickName() == targetNick)
                {
                    target = c;
                    break;
                }
            }
            if (target)
                it->kick(&client, target);
            else
                client.sendMessage("User   " + targetNick + " not found in the channel.");
        }
    }
    else if (cmd == "INVITE")
    {
        std::string channelName;
        std::string targetNick;
        iss >> channelName >> targetNick;

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), ChannelNameMatcher(channelName));

        if (it != channels.end())
        {
            if (it->isOperator(&client))
            {
                Client *target = NULL;
                std::vector<Client*> clientList = it->getClients();
                
                for (std:: vector<Client*>::iterator it = clientList.begin(); it != clientList.end(); ++it)
                {
                    Client *c = *it;
                    if (c->getnickName() == targetNick)
                    {
                        target = c;
                        break;
                    }
                }
                if (target)
                {
                    it->invite(target);
                    target->sendMessage("You have been invited to " + channelName);
                }
                else
                    client.sendMessage("User   " + targetNick + " not found.");
            }
            else
                client.sendMessage("You are not an operator in this channel.");
        }
        else
            client.sendMessage("Channel " + channelName + " not found.");
    }
    else if (cmd == "MODE")
    {
        std::string channelName;
        std::string mode;
        std::string value;
        iss >> channelName >> mode >> value;

        logger.logDebug(mode);
        if (mode.length() == 1)
        {
            std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), ChannelNameMatcher(channelName));

            if (it != channels.end())
            {
                if (it->isOperator(&client))
                {
                    if (mode == "k")
                        it->setMode(mode[0], static_cast<void*>(&value));
                    else if (mode == "l")
                    {
                        int userLimitValue = atoi(value.c_str());
                        it->setMode(mode[0], static_cast<void*>(&userLimitValue));
                    }
                    else
                    {
                        bool boolValue = (value == "true" || value == "1");
                        it->setMode(mode[0], static_cast<void*>(&boolValue));
                    }
                    it->broadcastMessage("Mode changed to: " + mode, &client);
                }
                else
                    client.sendMessage("You are not an operator in this channel.");
            }
        }
        else
            client.sendMessage("Invalid mode format. Must be a single character.");
    }
    else if (cmd == "QUIT")
    {
        std::string quitMessage;
        std::getline(iss, quitMessage);

        for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            if (it->isClientInChannel(&client))
            {
                std::string leaveMessage = client.getnickName() + " has left the channel: " + quitMessage;
                it->broadcastMessage(leaveMessage.c_str(), NULL);
                it->leave(&client);
            }
        }
        server.removeUser(client.getC_fd());
    }
    else if (cmd == "PING")
    {
        if (!commands[1].empty())
            client.sendMessage("PONG :" + commands[1]);
        else
            client.sendMessage("PONG :");
    }
    else
        client.sendMessage("Unknown command: " + cmd);
}

std::vector<std::string> Command::getTheCommand(std::string &command)
{
    std::vector<std::string> cmds;
    std::string currentWord;
    std::istringstream spliter(command);

    if (command.empty())
        return cmds;

    while (spliter >> currentWord)
    {
        if (currentWord[0] == ':')
        {
            currentWord.erase(0, 1);
            std::string restOfLine;
            std::getline(spliter, restOfLine); 
            currentWord += restOfLine;   
        }

        // Skip adding "CAP" or "LS" to the commands vector
        if (currentWord == "CAP" || currentWord == "LS")
            continue;
        cmds.push_back(currentWord);
    }
    return cmds;
}