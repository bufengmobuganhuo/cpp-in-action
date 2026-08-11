//
// Created by yuzhang on 2026/7/6.
//
#include "../include/OrderBookEntry.h"

OrderBookType OrderBookEntry::stringToOrderBookType(const std::string& s)
{
    if (s == "ask")
    {
        return OrderBookType::ask;
    }
    if (s == "bid")
    {
        return OrderBookType::bid;
    }
    return OrderBookType::unknown;
}
