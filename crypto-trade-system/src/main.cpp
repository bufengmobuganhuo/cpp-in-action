#include <iostream>
#include "../include/Wallet.h"
#include "../include/MerkelMain.h"

int main()
{
    MerkelMain merkel;
    merkel.init();

    // Wallet wallet;
    // wallet.insertCurrency("BTC", 10);
    // std::cout << "Wallet has BTC: " << wallet.containsCurrency("BTC", 10) << std::endl;
    // wallet.removeCurrency("BTC", 5);
    // std::cout << "Wallet has BTC: " << wallet.containsCurrency("BTC", 10) << std::endl;
    // wallet.removeCurrency("USDC", 5);
    // std::cout << wallet.toString() << std::endl;
}