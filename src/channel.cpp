#include "../includes/irc.hpp"

Channel::Channel(const std::string &name) : ch_name(name), inviteOnly(false), topicRestricted(false), userLimit(-1){}

void Channel::join(Client *client)
{
    if (userLimit != -1 && static_cast<int>(clients.size()) >= userLimit)
        return;
    clients.push_back(client);
    broadcastMessage(client->getnickName() + " has joined the channel.", NULL);
}

void Channel::leave(Client *client)
{
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    broadcastMessage(client->getnickName() + " has left the channel.", NULL);
}

void Channel::kick(Client *client, Client *target)
{
    if (isOperator(client))
    {
        leave(target);
        broadcastMessage(target->getnickName() + " has been kicked from the channel.", client);
    }
}

void Channel::invite(Client *target) { (void)target; }

void Channel::setTopic(const std::string &newTopic) { topic = newTopic; }

std::string Channel::getTopic() const { return topic; }

void Channel::broadcastMessage(const std::string &message, Client *sender)
{
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        Client *client = *it;
        if (client != sender)
            client->sendMessage(message);
    }
}

void Channel::setMode(char mode, void *value)
{
    switch (mode)
    {
        case 'i':
            inviteOnly = *static_cast<bool*>(value);
            break;
        case 't':
            topicRestricted = *static_cast<bool*>(value);
            break;
        case 'k':
            key = *static_cast<std::string*>(value);
            break;
        case 'o':
            {
                Client* client = *static_cast<Client**>(value);
                if (isOperator(client))
                    operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
                else
                    operators.push_back(client);
            }
            break;
        case 'l':
            userLimit = *static_cast<int*>(value);
            if (userLimit < 0)
                userLimit = -1;
            break;
        default:
            logger.logError("Unknown mode: " + mode);
            break;
    }
    std::string modeChangeMessage = "Mode changed: " + std::string(1, mode);
    broadcastMessage(modeChangeMessage, NULL);
}

std::vector<Client*> Channel::getClients() const { return clients; }

std::string Channel::getName() const { return ch_name; }

void Channel::setName(std::string name) { this->ch_name = name; }

bool Channel::isOperator(Client *client) const { return std::find(operators.begin(), operators.end(), client) != operators.end(); }

void Channel::setOperator(Client *client, bool isOp)
{
    if (isOp)
        operators.push_back(client);
    else
        operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
}