//
// Created by yuzhang on 2026/7/3.
//

#include "../include/CSVReader.h"

#include <fstream>
#include <iostream>
#include <vector>

CSVReader::CSVReader()
{
}

std::vector<OrderBookEntry> CSVReader::readCSV(std::string& csvFilename)
{
    std::vector<OrderBookEntry> entries;

    std::ifstream csvFile(csvFilename);
    std::string line;
    if (csvFile.is_open())
    {
        while (getline(csvFile, line))
        {
            try
            {
                OrderBookEntry obe = stringsToOBE(tokenise(line, ','));
                entries.push_back(std::move(obe));
            }
            catch (const std::exception& e)
            {
                std::cout << "CSVReader::readCSV bad data" << std::endl;
            }
        }
    }

    std::cout << "CSVReader::readCSV read " << entries.size() << " entries " << std::endl;
    return entries;
}

std::vector<std::string> CSVReader::tokenise(std::string& csvLine, char separator)
{
    std::vector<std::string> tokens;
    unsigned int start, end;
    std::string token;
    start = csvLine.find_first_not_of(separator, 0);
    do
    {
        end = csvLine.find_first_of(separator, start);
        if (start == csvLine.length() || start == end)
        {
            break;
        }
        if (end >= 0)
        {
            token = csvLine.substr(start, end - start);
        }
        else
        {
            token = csvLine.substr(start, csvLine.length() - start);
        }
        tokens.push_back(token);
        start = end + 1;
    }
    while (end > 0);

    return tokens;
}

OrderBookEntry CSVReader::stringsToOBE(std::string& price, std::string amount, std::string timestamp,
                                       std::string product, OrderBookType orderBookType)
{

}

OrderBookEntry CSVReader::stringsToOBE(const std::vector<std::string>& strings)
{
}
