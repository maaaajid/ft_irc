#include "../includes/irc.hpp"

void Command::handleCommand(const std::string &command, Client &client, std::vector<Channel> &channels)
{
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "JOIN")
    {
        std::string channelName;
        iss >> channelName;

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), [&channelName](Channel &channel)
        {
            return channel.getName() == channelName;
        });

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

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), [&channelName](Channel &channel)
        {
            return channel.getName() == channelName;
        });

        if (it != channels.end())
            it->leave(&client);
    }
    else if (cmd == "MSG")
    {
        std::string channelName;
        std::string message;
        iss >> channelName;
        std::getline(iss, message);
        
        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), [&channelName](Channel &channel)
        {
            return channel.getName() == channelName;
        });

        if (it != channels.end())
            it->broadcastMessage(client.getnickName() + ": " + message, &client);
    }
    else if (cmd == "TOPIC")
    {
        std::string channelName;
        std::string newTopic;
        iss >> channelName;
        std::getline(iss, newTopic);

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), [&channelName](Channel &channel)
        {
            return channel.getName() == channelName;
        });

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

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), [&channelName](Channel &channel)
        {
            return channel.getName() == channelName;
        });

        if (it != channels.end())
        {
            Client *target = nullptr;
            std::vector<Client*>clientList = it->getClients();
            
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
                client.sendMessage("User  " + targetNick + " not found in the channel.");
        }
    }
    else if (cmd == "INVITE")
    {
        std::string channelName;
        std::string targetNick;
        iss >> channelName >> targetNick;

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), [&channelName](Channel &channel)
        {
            return channel.getName() == channelName;
        });

        if (it != channels.end())
        {
            if (it->isOperator(&client))
            {
                Client *target = nullptr;
                std::vector<Client*>clientList = it->getClients();
                
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
                {
                    it->invite(target);
                    target->sendMessage("You have been invited to " + channelName);
                }
                else
                    client.sendMessage("User  " + targetNick + " not found.");
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

        std::vector<Channel>::iterator it = std::find_if(channels.begin(), channels.end(), [&channelName](Channel &channel)
        {
            return channel.getName() == channelName;
        });

        if (it != channels.end())
        {
            if (it->isOperator(&client))
            {
                it->setMode(mode, value);
                it->broadcastMessage("Mode changed to: " + mode, &client);
            }
            else
                client.sendMessage("You are not an operator in this channel.");
        }
    }
    else
        client.sendMessage("Unknown command: " + cmd);
}