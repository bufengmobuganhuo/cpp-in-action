//
// Created by yuzhang on 2026/7/3.
//

#ifndef CRYPTO_TRADE_SYSTEM_ORDERBOOK_H
#define CRYPTO_TRADE_SYSTEM_ORDERBOOK_H
#include <string>
#include <vector>

#include "OrderBookEntry.h"

class OrderBook
{
public:
    OrderBook(const std::string& filename);
    /**
     * return vector of all known products in the dataset
     */
    std::vector<std::string> getKnownProducts();
    /**
     * return vector of Orders according to the filters
     */
    std::vector<OrderBookEntry> getOrders(const OrderBookType& type, const std::string& product, const std::string& timestamp);
    /**
     * returns the earliest time in the orderbook
     */
    std::string getEarliestTime();
    void insertOrder(OrderBookEntry& entry);

    std::vector<OrderBookEntry> matchAsksToBids(std::string& product, std::string timestamp);

    static double getHighPrice(std::vector<OrderBookEntry>& orders);
    static double getLowPrice(std::vector<OrderBookEntry>& orders);
private:
    std::vector<OrderBookEntry> orders;
};
#endif //CRYPTO_TRADE_SYSTEM_ORDERBOOK_H
