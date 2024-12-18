#include "../includes/irc.hpp"

//toupper func 

std::string toupperfunc(std::string str)
{
    if (str.empty())
        return str;
    for (size_t i = 0; i < str.length(); i++)
    {
        str[i] = std::toupper(str[i]);
    }
    return str;
}


void Command::handleCommand(std::vector<std::string> &commands, Client &client, Server &server, epoll_event *events)
{
    try
    {
        if (commands.empty())
            return;

        std::string cmd = toupperfunc(commands[0]);
        if (!client.getAuth() && cmd != "PASS" && cmd != "NICK" && cmd != "USERHOST" && cmd != "USER" && cmd != "QUIT" && cmd != "PING" && cmd != "WHO" && cmd != "WHOIS")
            client.sendMessage("461 : You must complete authentication first.");
        else if (cmd == "PASS")
            client.passHandler(commands, server, events);
        else if (cmd == "NICK")
            client.nickHandler(commands[1], server);
        else if (cmd == "USERHOST" || cmd == "USER")
            client.userHandler(commands[1], server);
        else if (cmd == "JOIN")
        {
            if (commands.size() < 2)
            {
                // ERR_NEEDMOREPARAMS
                client.sendMessage("461 " + client.getnickName() + " JOIN :Not enough parameters");
                return;
            }
            std::string channelName = commands[1];
            if (!isValidChannelName(channelName))
            {
                // ERR_BADCHANMASK
                client.sendMessage("476 " + client.getnickName() + " " + channelName + " :Invalid channel name");
                return;
            }
            Channel *channel = server.getChannelByName(channelName);

            if (channel)
            {
                if (channel->isInviteOnly() && !channel->isInvited(&client))
                {
                    // ERR_INVITEONLYCHAN
                    client.sendMessage("473 " + client.getnickName() + " " + channelName + " :Cannot join channel (+i)");
                    return;
                }
                if (channel->hasKey())
                {
                    if (commands.size() < 3 || commands[2] != channel->getKey())
                    {
                        // ERR_BADCHANNELKEY
                        client.sendMessage("475 " + client.getnickName() + " " + channelName + " :Cannot join channel (+k)");
                        return;
                    }
                }
                if (channel->isFullyOccupied())
                {
                    // ERR_CHANNELISFULL
                    client.sendMessage("471 " + client.getnickName() + " " + channelName + " :Cannot join channel (+l)");
                    return;
                }
                channel->join(&client);
                std::string joinMessage = ":" + client.getnickName() + "!" + 
                                      client.getuserName() + "@" + 
                                      client.getC_ip() + " JOIN :" + channelName;
                channel->broadcastMessage(joinMessage, NULL);
                std::string topic = channel->getTopic();
                if (!topic.empty())  // RPL_TOPIC
                    client.sendMessage("332 " + client.getnickName() + " " + channelName + " :" + topic);
                else // RPL_NOTOPIC
                    client.sendMessage("331 " + client.getnickName() + " " + channelName + " :No topic is set");

                std::string nameReply = "353 " + client.getnickName() + " = " + channelName + " :";
                std::vector<Client*> channelClients = channel->getClients();

                for (std::vector<Client*>::iterator it = channelClients.begin(); it != channelClients.end(); ++it)
                    nameReply += (*it)->getnickName() + " ";
                client.sendMessage(nameReply);

                client.sendMessage("366 " + client.getnickName() + " " + channelName + " :End of /NAMES list");
            }
            else
            {
                logger.logInfo("Channel " + channelName + " does not exist. Creating a new one...");
                Channel *newChannel= new Channel(channelName);
                newChannel->join(&client);
                newChannel->addOperator(&client);
                server.addChannel(newChannel);

                std::string createMessage = ":" + client.getnickName() + "!" + 
                                        client.getuserName() + "@" + 
                                        client.getC_ip() + " JOIN :" + channelName;
                newChannel->broadcastMessage(createMessage, NULL);
                client.sendMessage("353 " + client.getnickName() + " = " + channelName + " :" + client.getnickName());
                client.sendMessage("366 " + client.getnickName() + " " + channelName + " :End of /NAMES list");
                client.sendMessage("331 " + client.getnickName() + " " + channelName + " :No topic is set");
            }
        }
        else if (cmd == "PART")
        {
            if (commands.size() < 2)
            {
                // ERR_NEEDMOREPARAMS
                client.sendMessage("461 " + client.getnickName() + " PART :Not enough parameters");
                return;
            }

            std::string channelName = commands[1];
            std::string partMessage = "";

            if (commands.size() > 2)
                partMessage = commands[2];

            Channel *channel = server.getChannelByName(channelName);

            if (!channel)
            {
                // ERR_NOSUCHCHANNEL
                client.sendMessage("403 " + client.getnickName() + " " + channelName + " :No such channel");
                return;
            }

            if (!channel->isClientInChannel(&client))
            {
                // ERR_NOTONCHANNEL
                client.sendMessage("442 " + client.getnickName() + " " + channelName + " :You're not on that channel");
                return;
            }
            std::string fullPartMessage = ":" + client.getnickName() + "!" + 
                                          client.getuserName() + "@" + 
                                          client.getC_ip() + " PART " + 
                                          channelName + 
                                          (partMessage.empty() ? "" : " :" + partMessage);
            channel->broadcastMessage(fullPartMessage, NULL);
            channel->leave(&client);
            if (channel->getClients().empty())
                server.removeChannel(channelName);
        }
        else if (cmd == "PRIVMSG")
        {
            if (commands.size() < 3)
            {
                client.sendMessage("ERROR :Not enough parameters");
                return;
            }

            std::string target = commands[1];
            std::string message = commands[2];
            if (target[0] == '#')
            {
                Channel *channel = server.getChannelByName(target);
                if (channel)
                {
                    std::string fullMessage = ":" + client.getnickName() + 
                                   "!" + client.getuserName() + 
                                   "@" + client.getC_ip() + 
                                   " PRIVMSG " + channel->getName() + " :" + message;
                    channel->broadcastMessage(fullMessage, &client);
                }
                else
                    client.sendMessage("ERROR :No such channel " + target);
            }
            else
            {
                Client *targetClient = NULL;
                std::vector<Client*> clients = server.getUsersList();
                for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
                {
                    if ((*it)->getnickName() == target)
                    {
                        targetClient = &(*(*it));
                        break;
                    }
                }
                if (targetClient)
                {
                    std::string fullMessage = ":" + client.getnickName() + 
                                      "!" + client.getuserName() + 
                                      "@" + client.getC_ip() + 
                                      " PRIVMSG " + targetClient->getnickName() + " :" + message;
                    targetClient->sendMessage(fullMessage);
                }
                else
                    client.sendMessage("ERROR :No such nick/channel " + target);
            }
        }
        else if (cmd == "TOPIC")
        {
            std::string channelName = commands[1];
            std::string newTopic = commands[2];

            Channel *channel = server.getChannelByName(channelName);
            if (channel)
            {
                if (channel->isOperator(&client))
                    channel->setTopic(newTopic, &client);
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
                    std::vector<Client*> clientList = server.getUsersList();

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
                        channel->invite(&client, target);
                    else
                        client.sendMessage("User " + targetNick + " not found.");
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

            std::vector<Channel*> channels = server.getChannels();
            for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end();)
            {
                if ((*it)->isClientInChannel(&client))
                {
                    std::string leaveMessage = client.getnickName() + " has left the channel: " + quitMessage;
                    (*it)->broadcastMessage(leaveMessage, NULL);
                    (*it)->leave(&client);
                    if ((*it)->getClients().empty())
                    {
                        std::string channelName = (*it)->getName();
                        server.removeChannel(channelName);
                        delete(*it);
                        it = channels.erase(it);
                        continue;
                    }
                }
                else
                    ++it;
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
        else if (cmd == "WHOIS" || cmd == "WHO")
            ;
        else
            Replies::NRP_UNKOWNCMD(client, cmd);
    }
    catch (const std::exception& e)
    {
        logger.logError("Command processing error: " + std::string(e.what()));
    }
}

std::vector<std::string> Command::getTheCommand(std::string &command)
{
    if (command.empty())
        return std::vector<std::string>();
    std::vector<std::string> cmds;
    std::string currentWord;
    std::stringstream spliter(command);


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
        modeStr = commands[2];

    if (commands.size() > 3)
        parameters = commands[3];

    bool isChannel = (target[0] == '#');

    if (isChannel)
    {
        Channel* channel = server.getChannelByName(target);
        if (!channel)
        {
            client.sendMessage("Channel " + target + " not found.");
            return;
        }

        if (modeStr.empty())
        {
            // RPL_CHANNELMODEIS
            std::string modeReply = "324 " + client.getnickName() + " " + target + " :Current modes";
            client.sendMessage(modeReply);
            return;
        }
        if (modeStr.length() < 2 || 
            (modeStr[0] != '+' && modeStr[0] != '-'))
        {
            client.sendMessage("Invalid mode format. Must be [+|-]modechars");
            return;
        }

        char operation = modeStr[0];  // + or -
        char mode = modeStr[1];       // actual mode character

        switch (mode)
        {
            case 'i':  // Invite-only mode
                channel->setMode(modeStr, "");
                break;
            case 't':  // Topic restriction mode
                channel->setMode(modeStr, "");
                break;
            case 'k':  // Channel key
                if (operation == '+' && !parameters.empty())
                    channel->setMode(modeStr, parameters);
                else if (operation == '-')
                    channel->setMode(modeStr, "");
                else
                {
                    client.sendMessage("Key mode requires a parameter when setting");
                    return;
                }
                break;
            case 'l':  // User limit
                if (operation == '+')
                {
                    int limit = atoi(parameters.c_str());
                    if (limit > 0)
                        channel->setMode(modeStr, parameters);
                    else
                    {
                        client.sendMessage("Invalid user limit");
                        return;
                    }
                }
                else if (operation == '-')
                    channel->setMode(modeStr, "-1");
                break;
            case 'o':  // Operator status
                if (!parameters.empty())
                {
                    if (channel->isOperator(&client))
                        channel->setMode(modeStr, parameters);
                    else
                        ;// client.sendMessage("You are not an operator");
                }
                else
                {
                    client.sendMessage("Operator mode requires a nickname");
                    return;
                }
                break;
            default:
                client.sendMessage("Unknown mode: " + std::string(1, mode));
                return;
        }

        std::string modeChangeMsg = ":" + client.getnickName() + 
                                    "!" + client.getuserName() + 
                                    "@" + client.getC_ip() + 
                                    " MODE " + target + " " + modeStr + 
                                    (parameters.empty() ? "" : " " + parameters);
        channel->broadcastMessage(modeChangeMsg, NULL);
    }
    else  // User mode
    {
        Client* targetClient = NULL;
        std::vector<Client*> clients = server.getUsersList();
        for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        {
            if ((*it)->getnickName() == target)
            {
                targetClient = &(*(*it));
                break;
            }
        }

        if (!targetClient)
        {
            client.sendMessage("User " + target + " not found.");
            return;
        }

        // Validate mode string
        if (modeStr.length() < 2 || 
            (modeStr[0] != '+' && modeStr[0] != '-'))
        {
            client.sendMessage("Invalid mode format. Must be [+|-]modechars");
            return;
        }

        char operation = modeStr[0];  // + or -
        char mode = modeStr[1];       // actual mode character

        switch (mode)
        {
            case 'i':  // Invisible mode
                if (operation == '+')
                    targetClient->setInvisible(true);
                else
                    targetClient->setInvisible(false);
                break;
            default:
                client.sendMessage("Unknown user mode: " + std::string(1, mode));
                return;
        }
    }
}