#include "../includes/irc.hpp"

Channel::Channel(const std::string &name) : ch_name(name), inviteOnly(false), topicRestricted(false), userLimit(-1){}

void Channel::join(Client *client)
{
    if (userLimit != -1 && clients.size() >= userLimit)
        return;
    clients.push_back(client);
    broadcastMessage(client->getnickName() + " has joined the channel.", nullptr);
}

void Channel::leave(Client *client)
{
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    broadcastMessage(client->getnickName() + " has left the channel.", nullptr);
}

void Channel::kick(Client *client, Client *target)
{
    if (isOperator(client))
    {
        leave(target);
        broadcastMessage(target->getnickName() + " has been kicked from the channel.", client);
    }
}

void Channel::invite(Client *target) {}

void Channel::setTopic(const std::string &newTopic) { topic = newTopic; }

std::string Channel::getTopic() const { return topic; }

void Channel::broadcastMessage(const std::string &message, Client *sender)
{
    for (Client *client : clients)
        if (client != sender)
            client->sendMessage(message);
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
            // Handle operator privilege logic
            break;
        case 'l':
            userLimit = *static_cast<int*>(value);
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