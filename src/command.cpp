#include "../includes/irc.hpp"

void Command::handleCommand(std::vector<std::string> &commands, Client &client, Server &server, epoll_event *events)
{
    try
    {
        if (commands.empty())
            return;

        std::string cmd = commands[0];

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
            std::string channelName = commands[1];

            Channel *channel = server.getChannelByName(channelName);

            if (channel)
                channel->join(&client);
            else
            {
                logger.logInfo("Channel " + channelName + " does not exist. Creating a new one...");
                Channel newChannel(channelName);
                newChannel.join(&client);
                server.addChannel(newChannel);
                newChannel.broadcastMessage("Channel " + channelName + " created by user " + client.getuserName(), &client);
            }
        }
        else if (cmd == "PART")
        {
            if (commands.size() < 2)
            {
                client.sendMessage("ERROR :Not enough parameters");
                return;
            }

            std::string channelName = commands[1];
            Channel *channel = server.getChannelByName(channelName);

            if (channel)
            {
                if (channel->isClientInChannel(&client))
                {
                    std::string partMessage = ":" + client.getnickName() + "!" + 
                                              client.getuserName() + "@" + 
                                              client.getC_ip() + " PART :" + channelName;
                    channel->broadcastMessage(partMessage, NULL);
                    channel->leave(&client);
                }
                else
                    client.sendMessage("ERROR :You are not in channel " + channelName);
            }
            else
                client.sendMessage("ERROR :No such channel " + channelName);
        }
        else if (cmd == "MSG")
        {
            std::string channelName = commands[1];
            std::string message = commands[2];

            Channel *channel = server.getChannelByName(channelName);

            if (channel)
                channel->broadcastMessage(client.getnickName() + ": " + message, &client);
            else
                logger.logWarning("Can't broadcast message. Channel " + channelName + " does not exist.");
        }
        else if (cmd == "TOPIC")
        {
            std::string channelName = commands[1];
            std::string newTopic = commands[2];

            Channel *channel = server.getChannelByName(channelName);
            if (channel)
            {
                if (channel->isOperator(&client))
                {
                    channel->setTopic(newTopic);
                    channel->broadcastMessage("New topic: " + newTopic, &client);
                }
                else
                    client.sendMessage("You are not an operator in this channel.");
            }
            else
                logger.logWarning("Can't change topic. Channel " + channelName + " does not exist.");
        }
        else if (cmd == "KICK")
        {
            std::string channelName = commands[1];
            std::string targetNick = commands[2];

            Channel *channel = server.getChannelByName(channelName);

            if (channel)
            {
                Client *target = NULL;
                std::vector<Client*> clientList = channel->getClients();

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
                    channel->kick(&client, target);
                else
                    client.sendMessage("User   " + targetNick + " not found in the channel.");
            }
            else
                logger.logWarning("Can't kick " + targetNick + ". Channel " + channelName + " does not exist.");
        }
        else if (cmd == "INVITE")
        {
            std::string channelName = commands[1];
            std::string targetNick = commands[2];

            Channel *channel = server.getChannelByName(channelName);
            if (channel)
            {
                if (channel->isOperator(&client))
                {
                    Client *target = NULL;
                    std::vector<Client*> clientList = channel->getClients();

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
                        channel->invite(target);
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
            std::string channelName = commands[1];
            std::string mode = commands[2];
            std::string value = commands[3];

            setModeHandler(commands, client, server);
        }
        else if (cmd == "QUIT")
        {
            std::string quitMessage = (commands.size() > 1) ? commands[1] : "Client has disconnected";

            std::vector<Channel>::iterator it;
            for (it = server.getChannels().begin(); it != server.getChannels().end(); ++it)
            {
                if (it->isClientInChannel(&client))
                {
                    std::string leaveMessage = client.getnickName() + " has left the channel: " + quitMessage;
                    it->broadcastMessage(leaveMessage, NULL);
                    it->leave(&client);
                }
            }
            server.removeUser(client.getC_fd(), events);
            return ;
        }
        else if (cmd == "PING")
        {
            if (!commands[1].empty())
                client.sendMessage("PONG :" + commands[1]);
            else
                client.sendMessage("PONG :");
        }
        else if (cmd == "NICK")
        {
            // client.changeNick(commands[1], server);
            
        }
        else if (cmd == "PASS" )
            ;
        else
            client.sendMessage("Unknown command: " + cmd);
    }
    catch (const std::exception& e)
    {
        logger.logError("Command processing error: " + std::string(e.what()));
        client.sendMessage("ERROR :Internal server error");
    }
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
                client.sendMessage("Unknown mode: " + std::string(1, mode));
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
                client.sendMessage("Unknown mode for user: " + std::string(1, mode));
        }
        else
            client.sendMessage("Invalid mode format. Must be [+|-][modechar]");
    }
}