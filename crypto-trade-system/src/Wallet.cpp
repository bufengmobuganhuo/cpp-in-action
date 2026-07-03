//
// Created by yuzhang on 2026/6/28.
//
#include "../include/Wallet.h"
#include <iostream>
#include "../include/CSVReader.h"

Wallet::Wallet()
{

}

void Wallet::insertCurrency(const std::string& type, double amount)
{
    double balance = 0;
    if (amount < 0)
    {
        throw std::exception{};
    }
    if (currencies.count(type) == 0)
    {
        balance = 0;
    }
    else
    {
        balance = currencies[type];
    }
    balance += amount;
    currencies[type] = balance;
}

bool Wallet::containsCurrency(const std::string& type, double amount)
{
    if (currencies.count(type) == 0)
    {
        return false;
    }
    return currencies[type] >= amount;
}

bool Wallet::removeCurrency(const std::string& type, double amount)
{
    if (amount < 0)
    {
        return false;
    }
    if (!containsCurrency(type, amount))
    {
        std::cout << "No enuough currency for " << type << std::endl;
        return false;
    }
    std::cout << "Removing " << type << " : " << amount << std::endl;
    currencies[type] -= amount;
    return true;
}

std::string Wallet::toString() const
{
    std::string s;
    for (std::pair<std::string, double> pair : currencies)
    {
        std:: string currency = pair.first;
        double amount = pair.second;
        s += currency + " : " + std::to_string(amount) + "\n";
    }
    return s;
}

bool Wallet::canFulfillOrder(OrderBookEntry& order)
{
    // product's format is "C1/C2", it means using C1 to exchange C2
    std::vector<std::string>  currencies = CSVReader::tokenise(order.product, '/');
    if (order.orderType == OrderBookType::ask)
    {
        // sell, check if it has enough C1 to sell.
        double amount = order.amount;
        const std::string& currency = currencies[0];
        return containsCurrency(currency, amount);
    }
    if (order.orderType == OrderBookType::bid)
    {
        // buy, check if it has enough C2 to buy
        double amount = order.amount * order.price;
        const std::string& currency = currencies[1];
        return containsCurrency(currency, amount);
    }
    return false;
}
