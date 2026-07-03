#include <fstream>
#include <iostream>
#include <string>
//
// Created by yuzhang on 2026/6/28.
//
#include "../include/billing.h"

void shopping::menu()
{
    int choice;
    string emial;
    string password;
    while (true)
    {
        system("cls");
        cout << "\t\t\t\t__________________________________\n";
        cout << "\t\t\t\t                                  \n";
        cout << "\t\t\t\t       Supermarket Main Menu      \n";
        cout << "\t\t\t\t                                  \n";
        cout << "\t\t\t\t__________________________________\n";

        cout << "\t\t\t\t|     1)Administrator  |\n";
        cout << "\t\t\t\t|                      |\n";
        cout << "\t\t\t\t|     2)Buyer          |\n";
        cout << "\t\t\t\t|                      |\n";
        cout << "\t\t\t\t|     3)Exit           |\n";
        cout << "\t\t\t\t|                      |\n";
        cout << "\n\t\t\t Please select!";
        cin>>choice;

        switch (choice)
        {
        case 1:
            cout << "\t\t\t Please login \n";
            cout << "\t\t\t Enter Email  \n";
            cin>>emial;
            cout << "\t\t\t Password     \n";
            cin>>password;

            if (emial == "robby@email.com" && password == "robby@123")
            {
                administrator();
            }
            else
            {
                cout << "Invalid email/password";
            }
            break;
        case 2:
            buyer();
            break;
        case 3:
            exit(0);
        default:
            cout << "Please select from the given seclections";
        }
    }
}

void shopping::administrator()
{
    int choice;
    while (true)
    {
        system("cls");
        cout << "\n\n\n\t\t\t Administrator menu";
        cout << "\n\t\t\t|______1) Add the product______|";
        cout << "\n\t\t\t|                              |";
        cout << "\n\t\t\t|______2) Modify the product___|";
        cout << "\n\t\t\t|                              |";
        cout << "\n\t\t\t|______3) Delete the product___|";
        cout << "\n\t\t\t|                              |";
        cout << "\n\t\t\t|______4) Back to main menu____|";

        cout << "\n\t\t\t Please select!";
        cin >> choice;
        switch (choice)
        {
        case 1:
            add();
            break;
        case 2:
            edit();
            break;
        case 3:
            rem();
            break;
        case 4:
            return;
        default:
            cout << "Please select from the given seclections";
        }
    }
}

void shopping::buyer()
{
    int choice;
    while (true)
    {
        system("cls");
        cout << "\n\n\n\t\t\t         Buyer menu         ";
        cout << "\n\t\t\t|______1) Buy product__________|";
        cout << "\n\t\t\t|                              |";
        cout << "\n\t\t\t|______3) Back to main menu____|";
        cout << "\n\t\t\t Please select!";

        cin >> choice;
        switch (choice)
        {
        case 1:
            receipt();
            break;
        case 2:
            return;
        default:
            cout << "Please select from the given seclections";
        }
    }
}

void shopping::add()
{
    fstream data;
    int c;
    int token = 0;
    float p;
    float d;
    string n;

    while (true)
    {
        system("cls");
        cout << "\n\n\n\t\t\t      Add new product         ";
        cout << "\n\t product code of the product: ";
        cin >> pcode;
        cout << "\n\t product name of the product: ";
        cin >> pname;
        cout << "\n\t product price of the product: ";
        cin >> price;
        cout << "\n\t product discount of the product: ";
        cin >> dis;

        data.open("database.txt", ios::in);
        if (!data)
        {
            data.open("database.txt", ios::app|ios::out);
            data << "  " << pcode << "  " << pname << "  " << price << "  " << dis << endl;
            data.close();
        }
        else
        {
            data >> c >> n >> p >> d;
            while (!data.eof())
            {
                if (c == pcode)
                {
                    token++;
                    break;
                }
                data >> c >> n >> p >> d;
            }
            data.close();
        }

        if (token > 0)
        {
            // 产品已存在
            cout << "product is already exists, add again" << endl;
        }
        else
        {
            // 写入新产品
            data.open("database.txt", ios::app|ios::out);
            data << "  " << pcode << "  " << pname << "  " << price << "  " << dis << endl;
            data.close();
            break;
        }
    }
    cout << "\t\tinserted new product" << endl;
}

void shopping::edit()
{
    fstream data, data1;
    int pkey;
    int token = 0;
    int c;
    float p;
    float d;
    string n;

    cout << "\n\t\t\t Modify the prodcut";
    cout << "\n\t\t\t Product code: ";
    cin >> pkey;

    data.open("database.txt", ios::in);
    if (!data)
    {
        cout << "\n\nFile doesn't exist";
    }
    else
    {
        data1.open("database1.txt", ios::app | ios::out);

        data >> pcode >> pname >> price >> dis;
        while (!data.eof())
        {
            if (pkey == pcode)
            {
                token++;
                cout << "\n\t\t product new code:";
                cin >> c;
                cout << "\n\t\t name of the product";
                cin >> n;
                cout << "\n\t\t price of the product";
                cin >> p;
                cout << "\n\t\t discount of the product";
                cin >> d;

                data1 << "  " << c << "  " << n << "  " << p << "  " << d << endl;

                cout << "\n\n\t\t prodcut edited";
            }
            else
            {
                data1 << "  " << pcode << "  " << pname << "  " << price << "  " << dis << endl;
            }
            data >> pcode >> pname >> price >> dis;
        }
        data.close();
        data1.close();

        // 删除文件
        remove("database.txt");
        // 重命名文件
        rename("database1.txt", "database.txt");
        if (token <= 0)
        {
            cout << "product not found";
        }
    }
}

void shopping::rem()
{
    fstream data, data1;
    int pkey;
    int token = 0;
    cout << "\n\t\t\t Modify the product";
    cout << "\n\t\t\t Product code: ";
    cin >> pkey;

    data.open("database.txt", ios::in);
    if (!data)
    {
        cout << "\n\nFile doesn't exist";
    }
    else
    {
        data1.open("database1.txt", ios::app | ios::out);

        data >> pcode >> pname >> price >> dis;
        while (!data.eof())
        {

            if (pcode != pkey)
            {
                data1 << "  " << pcode << "  " << pname << "  " << price << "  " << dis << endl;
            }
            else
            {
                token++;
                cout << "\n\n\t\t prodcut deleted";
            }
            data >> pcode >> pname >> price >> dis;
        }

        data.close();
        data1.close();

        remove("database.txt");
        rename("database1.txt", "database.txt");
        if (token <= 0)
        {
            cout << "product not found";
        }
    }
}

void shopping::list()
{
    fstream data;
    data.open("database.txt", ios::in);
    cout << "\n\n\t\t |_________________________________|" << endl;
    cout << "ProNo\t\tProName\t\tProPrice\t\tProDiscount" << endl;
    cout << "\n\n\t\t |_________________________________|" << endl;
    data >> pcode >> pname >> price >> dis;
    while (!data.eof())
    {
        cout << pcode << "\t\t" << pname << "\t\t" << price << "\t\t" << dis << endl;
        data >> pcode >> pname >> price >> dis;
    }
    data.close();
}

void shopping::receipt()
{
    fstream data;

    vector<int> lpcode;
    vector<int> lpquantity;

    char choice;
    int pcount = 0;
    float amount = 0;
    float total = 0;

    cout << "\n\n\t\t\t RECEIPT";
    data.open("database.txt", ios::in);
    if (!data)
    {
        cout << "\n\nNo product to buy";
    }
    else
    {
        data.close();
        list();
        cout << "\n______________________________________\n";
        cout << "\n";
        cout << "\n        Please place the order        \n";
        cout << "\n______________________________________\n";

        m:
        do
        {
            cout << "\n\nEnter Product code: ";
            cin >> pcode;
            for (int code : lpcode)
            {
                if (code == pcode)
                {
                    cout << "\n\nDuplicate product code: " << pcode;
                    goto m;;
                }
            }
            lpcode.push_back(pcode);
            cout << "\n\nEnter the product quantity: ";
            int quantity;
            cin >> quantity;
            lpquantity.push_back(quantity);
            pcount++;
            cout << "\n\nDo you want to buy another product";
            cin >> choice;

        }
        while (choice == 'y');
        cout << "\n\n\t\t\t RECEIPT";
        cout << "\nProNo\tProName\tProQuantity\tProPrice\tProAmount\tProAmount with discount";
        for (int i = 0; i < pcount; i++)
        {
            data.open("database.txt", ios::in);
            data >> pcode >> pname >> price >> dis;
            while (!data.eof())
            {
                if (pcode == lpcode.at(i))
                {
                    amount = price * lpquantity.at(i);
                    dis = dis * amount;
                    total += amount - dis;
                    cout << pcode << "\t" << pname << "\t" << lpquantity.at(i) << "\t" << amount << "\t" << dis << endl;
                    break;
                }
                data >> pcode >> pname >> price >> dis;
            }
            data.close();
        }
        cout << "cost total: " << total << endl;
    }
}