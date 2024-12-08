#include "../includes/irc.hpp"

Channel::Channel(const std::string &name) : ch_name(name), inviteOnly(false), topicRestricted(false), userLimit(-1){}

void Channel::join(Client *client)
{
    if (userLimit != -1 && static_cast<int>(clients.size()) >= userLimit)
        return;
    clients.push_back(client);
    broadcastMessage(client->getnickName() + " has joined the channel " + ch_name + ".", NULL);
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

void Channel::setMode(std::string mode, std::string value)
{
    switch (mode[1])
    {
        case 'i':
            inviteOnly = mode[0] == '+' ? true : false;
            break;
        case 't':
            topicRestricted = mode[0] == '+' ? true : false;
            break;
        case 'k':
            mode[0] == '+' ? key = value : key = "";
            break;
        case 'o':
        {
            Client* client = findClient(value);
            if (isOperator(client) && mode[0] == '-')
                operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
            else if (client && mode[0] == '+')
                operators.push_back(client);
            // else send message that the client is not in the channel
        }
            break;
        case 'l':
            userLimit = atoi(value.c_str());
            if (userLimit < 0)
                userLimit = -1;
            break;
        default:
            logger.logError("Unknown mode: " + mode);
            break;
    }
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

bool Channel::isClientInChannel(Client *client) const
{
    return std::find(clients.begin(), clients.end(), client) != clients.end();
}

Client* Channel::findClient(std::string name)
{
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        if ((*it)->getnickName() == name)
            return *it;
    return NULL;
}