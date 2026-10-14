#include <iostream>
#include "dataProcess.hpp"

int main()
{
    const std::string DATA_DIR = "data/";   // csv数据统一存放目录
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

        if(srcPinyin == "0" && dstPinyin == "0") break;    // 退出机制

        int src = FindAirport(airports, srcPinyin);     // 起始点
        int dst = FindAirport(airports, dstPinyin);     // 目的地
        if(src < 0 || dst < 0) continue;

        QueryDirect(flights, src, dst);
    }
    return 0;
}