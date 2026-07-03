#include <utility>

//
// Created by yuzhang on 2026/6/30.
//

#ifndef CRYPTO_TRADE_SYSTEM_ORDERBOOKENTRY_H
#define CRYPTO_TRADE_SYSTEM_ORDERBOOKENTRY_H

enum class OrderBookType
{
    bid,
    ask,
    unknown,
    asksale,
    bidsale
};

class OrderBookEntry
{
public:
    double price;
    double amount;
    std::string timestamp;
    std::string product;
    OrderBookType orderType;
    std::string username;

    OrderBookEntry(double _price,
        double _amount,
        std::string _timestamp,
        std::string _product,
        OrderBookType _orderType,
        std::string username = "dataset")
            :price(_price), amount(_amount), timestamp(std::move(_timestamp)),
             product(std::move(_product)), orderType(_orderType), username(std::move(username)){};

    OrderBookEntry(OrderBookEntry&&) = default;
    OrderBookEntry& operator=(OrderBookEntry&&) = default;

    static OrderBookType stringToOrderBookType(std::string& s);

    static bool compareByTimestamp(OrderBookEntry& e1, OrderBookEntry& e2)
    {
        return e1.timestamp < e2.timestamp;
    }

    static bool compareByPriceAsc(OrderBookEntry& e1, OrderBookEntry& e2)
    {
        return  e1.price < e2.price;
    }

    static bool compareByPriceDesc(OrderBookEntry& e1, OrderBookEntry& e2)
    {
        return e2.price < e2.price;
    }
};
#endif //CRYPTO_TRADE_SYSTEM_ORDERBOOKENTRY_H
