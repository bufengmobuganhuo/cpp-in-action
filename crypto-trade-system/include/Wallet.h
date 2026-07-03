//
// Created by yuzhang on 2026/6/28.
//

#ifndef CRYPTO_TRADE_SYSTEM_WALLET_H
#define CRYPTO_TRADE_SYSTEM_WALLET_H
#include <string>
#include <map>
#include "OrderBookEntry.h"

class Wallet
{
public:
    Wallet();
    /**
     * insert currency to the wallet
     */
    void insertCurrency(const std::string& type, double amount);
    /**
     * remove currency from the wallet
     */
    bool removeCurrency(const std::string& type, double amount);
    /**
     * check if the wallet contains this much currency or more
     */
    [[nodiscard]] bool containsCurrency(const std::string& type, double amount);
    /**
     * generate a string representation of the wallet
     */
    [[nodiscard]] std::string toString() const;
    /**
     * check if the wallet can cope with this ask or bid
     */
    bool canFulfillOrder(OrderBookEntry& order);
private:
    /**
     * currency -> amount
     */
    std::map<std::string, double> currencies;
};
#endif //CRYPTO_TRADE_SYSTEM_WALLET_H
