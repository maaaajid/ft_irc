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
        std::vector<Client*> invitedClients;
        std::string topic;
        std::string key;
        bool inviteOnly;
        bool topicRestricted;
        int userLimit;

    public:
        Channel(const std::string &name);
        ~Channel();

        Channel& operator=(const Channel& other);
    
        //Commands
        void join(Client *client);
        void leave(Client *client);
        void kick(Client *client, Client *target);
        void invite(Client *target);
        void addOperator(Client* client);
        void removeOperator(Client* client);
        
        //Getters
        std::string getTopic() const;
        std::string getName() const;
        std::vector<Client*> getClients() const;
        std::string getKey() const;
        
        //Setters
        void setName(std::string name);
        void setMode(std::string mode, std::string value);
        void setOperator(Client *client, bool isOp);
        void setTopic(const std::string &newTopic);
        
        //Utils
        void broadcastMessage(const std::string &message, Client *sender);
        Client *findClient(std::string name);
        
        //Checkers
        bool isInviteOnly() const;
        bool isInvited(Client* client) const;
        bool hasKey() const;
        bool isClientInChannel(Client *client) const;
        bool isOperator(Client *client) const;
        bool isFullyOccupied() const;
};

#endif