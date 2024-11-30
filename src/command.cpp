#include "../includes/irc.hpp"

// Function object for matching channel names
struct ChannelNameMatcher {
    std::string channelName;
    ChannelNameMatcher(const std::string &name) : channelName(name) {}
    bool operator()(Channel &channel) {
        return channel.getName() == channelName;
    }
};

void Command::handleCommand(std::vector<std::string> &commands, Client &client, Server &server, epoll_event *events)
{
    if (commands.empty())
        return;

    vector<Channel>channels = server.getChannels();
    std::string cmd = commands[0];
    std::stringstream iss;
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
            channels.push_back(newChannel);
            newChannel.join(&client);
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

        setModeHandler(commands, client, server);
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
        server.removeUser(client.getC_fd(), events);
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
    std::stringstream spliter(command);

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

void Command::setModeHandler(std::vector<std::string> &commands, Client &client, Server &server)
{
    if (commands.size() < 2)
    {
        client.sendMessage("Usage: /mode <channel|nickname> [[+|-]modechars [parameters]]");
        return;
    }

    std::string target = commands[1];
    std::string modeStr;
    std::string parameters;

    if (commands.size() > 2)
    {
        modeStr = commands[2];
        if (commands.size() > 3)
            parameters = commands[3];
    }

    bool isChannel = (target[0] == '#');

    if (isChannel)
    {
        Channel *channel = server.getChannelByName(target);
        if (!channel)
        {
            client.sendMessage("Channel " + target + " not found.");
            return;
        }

        if (modeStr.length() == 2)
        {
            char mode = modeStr[1];
            if (mode == 'i' || mode == 'k' || mode == 't' || mode == 'o')
            {
                bool enable = (modeStr[0] == '+');
                if (mode == 'i' || mode == 't')
                    channel->setMode(modeStr, toString(modeStr[0]));
                else
                    channel->setMode(modeStr, parameters);
                std::string modeAction = (enable ? "enabled" : "disabled");
                channel->broadcastMessage("Mode " + modeStr + " has been " + modeAction + " in " + target, NULL);
            }
            else
                client.sendMessage("Unknown mode: " + mode);
        }
        else
            client.sendMessage("Invalid mode format. Must be [+|-][modechar]");
    }
    else
    {
        std::vector<Client> clients = server.getUsersList();
        Client *targetClient = NULL;
        for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
            if (it->getnickName() == target)
                targetClient = &(*it);
        if (!targetClient) {
            client.sendMessage("User  " + target + " not found.");
            return;
        }

        if (modeStr.length() == 2)
        {
            char mode = modeStr[1];
            if (mode == 'i') {
                bool enable = (modeStr[0] == '+');
                if (enable)
                {
                    targetClient->setInvisible(true);
                    client.sendMessage("User  " + target + " has been set invisible.");
                }
                else
                {
                    targetClient->setInvisible(false);
                    client.sendMessage("User  " + target + " has been set visible.");
                }
            }
            else
                client.sendMessage("Unknown mode for user: " + mode);
        }
        else
            client.sendMessage("Invalid mode format. Must be [+|-][modechar]");
    }
}