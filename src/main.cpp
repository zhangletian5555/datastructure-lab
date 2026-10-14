#include <iostream>
#include "dataProcess.hpp"

int main()
{
    const std::string DATA_DIR = "data/";
    Vector<flightInfo> flights = ReadInfo(DATA_DIR + "flight-data.csv");
    Vector<IDmapInfo> airports = ReadMap(DATA_DIR + "id2name.csv");

    if(flights.empty() || airports.empty())
    {
        std::cerr << "Data load failed.\n"; 
        return 1;
    }

    Statistics(flights);

    std::string line;
    while(std::getline(std::cin, line))
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::istringstream iss(line);
        std::string srcPinyin, dstPinyin;
        if(!(iss >> srcPinyin >> dstPinyin)) continue;

        if(srcPinyin == "0" && dstPinyin == "0") break;

        int src = FindAirport(airports, srcPinyin);
        int dst = FindAirport(airports, dstPinyin);
        if(src < 0 || dst < 0) continue;

        QueryDirect(flights, src, dst);
    }
    return 0;
}