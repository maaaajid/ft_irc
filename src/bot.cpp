#include "../includes/irc.hpp"

Bot::Bot() { std::srand(static_cast<unsigned int>(std::time(0))); }
Bot::~Bot() {}

std::string Bot::generateQuote()
{
    std::vector<std::string> quotes;

    quotes.push_back("The only limit to our realization of tomorrow is our doubts of today.");
    quotes.push_back("The future belongs to those who believe in the beauty of their dreams.");
    quotes.push_back("Do not wait to strike till the iron is hot, but make it hot by striking.");
    quotes.push_back("Success is not the key to happiness. Happiness is the key to success.");
    quotes.push_back("What lies behind us and what lies before us are tiny matters compared to what lies within us.");
    quotes.push_back("The best way to predict the future is to invent it.");
    quotes.push_back("The biggest risk is not taking any risk...");
    quotes.push_back("We cannot solve problems with the kind of thinking we employed when we came up with them.");
    quotes.push_back("The biggest adventure you can take is to live the life of your dreams.");
    quotes.push_back("Learn as if you will live forever, live like you will die tomorrow.");
    quotes.push_back("When you change your thoughts, remember to also change your world.");
    quotes.push_back("It is better to fail in originality than to succeed in imitation.");
    quotes.push_back("I never dreamed about success. I worked for it.");

    int randomIndex = std::rand() % quotes.size();
    return quotes[randomIndex];
}