//
// Created by yuzhang on 2026/6/28.
//

#ifndef SUPERMARKET_BILLING_SYSTEM_BILLING_H
#define SUPERMARKET_BILLING_SYSTEM_BILLING_H
#include <string>
using namespace std;

class shopping
{
private:
    int pcode;
    float price;
    float dis;
    string pname;

public:
    void menu();
    void administrator();
    void buyer();
    void add();
    void edit();
    void rem();
    void list();
    void receipt();
};

#endif //SUPERMARKET_BILLING_SYSTEM_BILLING_H
