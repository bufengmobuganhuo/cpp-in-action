//
// Created by yuzhang on 2026/6/28.
//

#ifndef CRYPTO_TRADE_SYSTEM_MARKETMAIN_H
#define CRYPTO_TRADE_SYSTEM_MARKETMAIN_H
#include <string>

#include "OrderBook.h"
#include "Wallet.h"

class MerkelMain
{
public:
    MerkelMain();
    void init();
private:
    void printMenu();
    void printHelp();
    void printMarketStats();
    void enterAsk();
    void enterBid();
    void printWallet();
    void gotoNextTimeframe();
    int getUserOperation();
    void processUserOperation(int userOption);

    std::string currentTime;

    OrderBook orderBook{"20200317.csv"};

    Wallet wallet;
};
#endif //CRYPTO_TRADE_SYSTEM_MARKETMAIN_H
