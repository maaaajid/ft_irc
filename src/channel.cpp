#include "../includes/irc.hpp"

Channel::Channel(const std::string &name) : ch_name(name), inviteOnly(false), topicRestricted(false), userLimit(-1) {}

Channel& Channel::operator=(const Channel& other)
{
            if (this != &other)
            {
                for (size_t i = 0; i < clients.size(); ++i)
                    delete clients[i];
                clients.clear();
                this->ch_name = other.ch_name;
                // Deep copy of clients
                for (size_t i = 0; i < other.clients.size(); ++i) {
                    this->clients.push_back(new Client(*other.clients[i])); // Allocate new Client
                }
            }
            return *this;
}
void Channel::join(Client *client)
{
    if (userLimit != -1 && static_cast<int>(clients.size()) >= userLimit)
        return;
    clients.push_back(client);
}

void Channel::leave(Client *client)
{
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    if (find(operators.begin(), operators.end(), client) != operators.end())
        operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
    if (find(invitedClients.begin(), invitedClients.end(), client) != invitedClients.end())
        invitedClients.erase(std::remove(invitedClients.begin(), invitedClients.end(), client), invitedClients.end());

    broadcastMessage(client->getnickName() + " has left the channel.", NULL);
}

void Channel::kick(Client *client, Client *target)
{
    if (isOperator(client))
    {
        std::string kickMessage = ":" + client->getnickName() + "!" + 
                                  client->getuserName() + "@" + 
                                  client->getC_ip() + " KICK " + 
                                  ch_name + " " + target->getnickName();
        broadcastMessage(kickMessage, NULL);
        leave(target);
        target->sendMessage("ERROR :You have been kicked from " + getName() + " by " + client->getnickName());
    }
}

void Channel::invite(Client *inviter, Client *target)
{
    std::string inviteReply = "341 " + inviter->getnickName() + " " + 
                               target->getnickName() + " " + ch_name;
    inviter->sendMessage(inviteReply);

    std::string inviteMessage = ":" + inviter->getnickName() + "!" + 
                                 inviter->getuserName() + "@" + 
                                 inviter->getC_ip() + " INVITE " + 
                                 target->getnickName() + " :" + ch_name;
    target->sendMessage(inviteMessage);
    invitedClients.push_back(target);
}

void Channel::setTopic(const std::string &newTopic, Client *setter) 
{
    topic = newTopic;
    std::string topicMessage = ":" + setter->getnickName() + "!" + 
                                setter->getuserName() + "@" + 
                                setter->getC_ip() + " TOPIC " + 
                                getName() + " :" + newTopic;
    broadcastMessage(topicMessage, NULL);
    std::string numericReply = "332 " + setter->getnickName() + " " + 
                                ch_name + " :" + newTopic;
    setter->sendMessage(numericReply);
}

std::string Channel::getTopic() const { return topic; }

void Channel::broadcastMessage(const std::string &message, Client *sender)
{
    std::vector<Client*> clientsCopy = clients;

    (void)sender;
    for (std::vector<Client*>::iterator it = clientsCopy.begin(); it != clientsCopy.end(); ++it)
    {
        Client* client = *it;

        if (client == NULL)
        {
            logger.logWarning("Null client in broadcast");
            continue;
        }

        try {
            if (client != sender)
                client->sendMessage(message);
        }
        catch (const std::exception& e) {
            logger.logError("Error broadcasting message: " + std::string(e.what()));
        }
    }
}

void Channel::setMode(std::string mode, std::string value)
{
    char operation = mode[0];  // + or -
    char modeChar = mode[1];   // actual mode character

    switch (modeChar)
    {
        case 'i':  // Invite-only
            inviteOnly = (operation == '+');
            break;
        case 't':  // Topic restriction
            topicRestricted = (operation == '+');
            break;
        case 'k':  // Channel key
            if (operation == '+')
                key = value;
            else
                key.clear();
            break;
        case 'l':  // User limit
            userLimit = (operation == '+') ? atoi(value.c_str()) : -1;
            break;
        case 'o':  // Operator status
        {
            Client* client = findClient(value);
            if (client)
            {
                if (operation == '+')
                    addOperator(client);
                else
                    removeOperator(client);
            }
            break;
        }
    }
}

std::vector<Client*> Channel::getClients() const { return clients; }

std::string Channel::getName() const { return ch_name; }

void Channel::setName(std::string name) { this->ch_name = name; }

bool Channel::isOperator(Client *client) const
{
    if (client == NULL)
        return false;

    for (std::vector<Client*>::const_iterator it = operators.begin(); it != operators.end(); ++it)
        if (*it == client)
            return true;
    return false;
}

void Channel::setOperator(Client *client, bool isOp)
{
    if (isOp)
        addOperator(client);
    else
        operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
}

bool Channel::isClientInChannel(Client *client) const
{
    if (client == NULL || clients.empty())
        return false;
    for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
        if (*it == client)
            return true;
    return false;
}

Client* Channel::findClient(std::string name)
{
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        if ((*it)->getnickName() == name)
            return *it;
    return NULL;
}

bool Channel::isInviteOnly() const { return inviteOnly; }

bool Channel::isInvited(Client* client) const
{
    for (std::vector<Client*>::const_iterator it = invitedClients.begin(); it != invitedClients.end(); ++it)
        if (*it == client)
            return true;
    return false;
}

bool Channel::hasKey() const { return !key.empty(); }

std::string Channel::getKey() const { return key; }

bool Channel::isFullyOccupied() const { return userLimit != -1 && static_cast<int>(clients.size()) >= userLimit; }

void Channel::addOperator(Client* client)
{
    if (client == NULL)
    {
        logger.logWarning("Attempting to add NULL client as operator");
        return;
    }

    if (!isClientInChannel(client))
    {
        logger.logWarning("Cannot add operator: Client not in channel");
        return;
    }

    if (!isOperator(client))
    {
        operators.push_back(client);
        std::string message = ":" + client->getnickName() + "!" + client->getuserName() + "@" + client->getC_ip() + " " + "MODE " + ch_name + " +o " + client->getnickName();
        broadcastMessage(message, NULL);
        std::string numericReply = "381 " + client->getnickName() + " :You are now an operator in " + ch_name;
        client->sendMessage(numericReply);
    }
}

void Channel::removeOperator(Client* client)
{
    operators.erase(
        std::remove(operators.begin(), operators.end(), client), 
        operators.end()
    );
}

Channel::~Channel()
{
    logger.logDebug("Destructed channel");
}