//
// Created by yuzhang on 2026/6/30.
//

#ifndef CRYPTO_TRADE_SYSTEM_CSVREADER_H
#define CRYPTO_TRADE_SYSTEM_CSVREADER_H
#include "OrderBookEntry.h"
#include <vector>
#include  <string>

class CSVReader
{
public:
    CSVReader();

    static std::vector<OrderBookEntry> readCSV(std::string& csvFile);
    static std::vector<std::string> tokenise(std::string& csvLine, char separator);
    static OrderBookEntry stringsToOBE(std::string& price,
        std::string amount,
        std::string timestamp,
        std::string product,
        OrderBookType orderBookType);

private:
    static OrderBookEntry stringsToOBE(const std::vector<std::string>& strings);
};


#endif //CRYPTO_TRADE_SYSTEM_CSVREADER_H
