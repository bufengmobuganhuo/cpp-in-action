//
// Created by yuzhang on 2026/7/6.
//
#include "../include/OrderBook.h"

#include <iostream>
#include <map>
#include "../include/CSVReader.h"

OrderBook::OrderBook(const std::string& filename)
{
    orders = CSVReader::readCSV(filename);
}

std::vector<std::string> OrderBook::getKnownProducts()
{
    std::vector<std::string> products;

    std::map<std::string, bool> prod_map;

    for (OrderBookEntry& order : orders)
    {
        prod_map[order.product] = true;
    }

    products.reserve(prod_map.size());
    for (const auto& e : prod_map)
    {
        products.push_back(e.first);
    }

    return products;
}

std::vector<OrderBookEntry> OrderBook::getOrders(const OrderBookType& type, const std::string& product,
                                                 const std::string& timestamp)
{
    std::vector<OrderBookEntry> orders_sub;
    for (OrderBookEntry& order_book_entry : orders)
    {
        if (order_book_entry.orderType == type && order_book_entry.product == product && order_book_entry.timestamp ==
            timestamp)
        {
            orders_sub.push_back(order_book_entry);
        }
    }
    return orders_sub;
}

double OrderBook::getHighPrice(std::vector<OrderBookEntry>& orders)
{
    double max = orders[0].price;
    for (OrderBookEntry& order : orders)
    {
        if (order.price > max)
        {
            max = order.price;
        }
    }
    return max;
}

double OrderBook::getLowPrice(std::vector<OrderBookEntry>& orders)
{
    double min = orders[0].price;
    for (OrderBookEntry& order : orders)
    {
        min = std::min(min, order.price);
    }
    return min;
}

std::string OrderBook::getEarliestTime()
{
    return orders[0].timestamp;
}

void OrderBook::insertOrder(OrderBookEntry& entry)
{
    orders.push_back(entry);
    std::sort(orders.begin(), orders.end(), OrderBookEntry::compareByTimestamp);
}

std::vector<OrderBookEntry> OrderBook::matchAsksToBids(std::string& product, std::string timestamp)
{
    std::vector<OrderBookEntry> asks = getOrders(OrderBookType::ask, product, timestamp);
    std::vector<OrderBookEntry> bids = getOrders(OrderBookType::bid, product, timestamp);

    std::vector<OrderBookEntry> sales;

    if (asks.size() == 0 || bids.size() == 0)
    {
        std::cout << "OrderBook::matchAsksToBids no bids or asks" << std::endl;
        return sales;
    }
    std::sort(asks.begin(), asks.end(), OrderBookEntry::compareByPriceAsc);
    std::sort(bids.begin(), bids.end(), OrderBookEntry::compareByPriceDesc);

    std::cout << "max ask " << asks[asks.size() - 1].price << std::endl;
    std::cout << "min ask " << asks[0].price << std::endl;
    std::cout << "max bid " << bids[0].price << std::endl;
    std::cout << "min bid " << bids[bids.size() - 1].price << std::endl;

    for (auto& ask : asks)
    {
        for (auto & bid : bids)
        {
            if (bid.price >= ask.price)
            {
                OrderBookEntry sale{ask.price, 0, timestamp, product, OrderBookType::asksale};
                if (bid.username == "simuser")
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::bidsale;
                }
                if (ask.username == "simuser")
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::asksale;
                }

                if (bid.amount == ask.amount)
                {
                    sale.amount = bid.amount;
                    sales.push_back(sale);
                    bid.amount = 0.0;
                    break;
                }
                if (bid.amount > ask.amount)
                {
                    sale.amount = ask.amount;
                    sales.push_back(sale);
                    bid.amount -= ask.amount;
                    break;
                }
                if (bid.amount < ask.amount && bid.amount > 0)
                {
                    sale.amount = bid.amount;
                    sales.push_back(sale);
                    ask.amount -= bid.amount;
                    bid.amount = 0;
                }
            }
        }
    }
    return sales;
}
