#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "irc.hpp"

class Client;

class Channel
{
    private:
        std::string ch_name;
        std::vector<Client*> clients;
        std::vector<Client*> operators;
        std::string topic;
        bool inviteOnly;
        bool topicRestricted;
        std::string key;
        int userLimit;

    public:
        Channel(const std::string &name);

        void join(Client *client);
        void leave(Client *client);
        void kick(Client *client, Client *target);
        void invite(Client *target);
        void setTopic(const std::string &newTopic);
        std::string getTopic() const;
        std::string getName() const;
        void setName(std::string name);
        void setMode(std::string mode, std::string value);
        bool isOperator(Client *client) const;
        void setOperator(Client *client, bool isOp);
        bool isClientInChannel(Client *client) const;
        
        void broadcastMessage(const std::string &message, Client *sender);
        std::vector<Client*> getClients() const;
        Client *findClient(std::string name);
};

#endif