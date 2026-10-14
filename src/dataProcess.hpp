#pragma once
#include "Vector.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <algorithm>

std::time_t ParseTime(const std::string &t);

struct flightInfo
{
    int flightID;
    int airFares;
    int flightNO;
    int airplaneModel;
    std::string departureTimeStr, arrivalTimeStr;
    std::time_t departureTime, arrivalTime;
    int departureAirport, arrivalAirport;
    int airplaneID;
    std::string departureDate;

    flightInfo(int Fid, int fares, int NO, int model, std::string depTime, std::string arrTime,
         int depPort, int arrPort, int Aid, std::string depDate) :
         flightID(Fid), airFares(fares), flightNO(NO), airplaneModel(model), 
         departureTimeStr(depTime), arrivalTimeStr(arrTime), 
         departureAirport(depPort), arrivalAirport(arrPort),
         airplaneID(Aid), departureDate(depDate) 
    {
        departureTime = ParseTime(depTime);
        arrivalTime = ParseTime(arrTime);
    }
    flightInfo() = default;
};

struct IDmapInfo
{
    int ID;
    std::string Pinyin;

    IDmapInfo(int id, std::string name) : ID(id), Pinyin(name) {}
    IDmapInfo() = default;
};

std::time_t ParseTime(const std::string &t)
{
    std::tm tm = {};
    std::istringstream ss(t);
    ss >> std::get_time(&tm, "%m/%d/%Y %H:%M");
    if(ss.fail())
        { std::cerr << "Fail to parse time!" << std::endl; return -1; }
    return std::mktime(&tm);
}

Vector<flightInfo> ReadInfo(const std::string &filename)
{
    Vector<flightInfo> flights;
    std::ifstream file(filename);
    if(!file.is_open())
    {
        std::cerr << "Fail to load file " << filename  << " !" << std::endl;
        return flights;
    }

    std::string line;
    std::getline(file, line);

    while(std::getline(file, line))
    {
        if(line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::stringstream ss(line);
        std::string field;
        Vector<std::string> fields;

        while (std::getline(ss, field, ',')) 
            fields.push_back(field);

        if(fields.size() < 10)
        {
            std::cerr << "Broken file!" << std::endl;
            return flights; 
        }

        try {
            int flightID    = std::stoi(fields[0]);  // flight ID
            int fares       = std::stoi(fields[1]);  // Air Fares
            int flightNO    = std::stoi(fields[2]);  // Flight NO.
            int model       = std::stoi(fields[3]);  // Airplane Model
            std::string depTimeStr = fields[4];      // Departure Time
            std::string arrTimeStr = fields[5];      // Arrival Time
            int depPort     = std::stoi(fields[6]);  // Departure Airport
            int arrPort     = std::stoi(fields[7]);  // Arrival Airport
            int airplaneID  = std::stoi(fields[8]);  // Airplane ID
            std::string depDate = fields[9];         // Departure Date

            flightInfo info(flightID, fares, flightNO, model, depTimeStr, arrTimeStr,
                            depPort, arrPort, airplaneID, depDate);
            flights.push_back(info);
        } catch (const std::exception &e){
            std::cerr << "Fail to initialize in line! error: " << e.what() << std::endl;
        }
    }

    file.close();
    return flights;
}

Vector<IDmapInfo> ReadMap(const std::string &filename)
{
    Vector<IDmapInfo> Id2Name;
    std::ifstream file(filename);
    if(!file.is_open())
    {
        std::cerr << "Fail to load file " << filename  << " !" << std::endl;
        return Id2Name;
    }

    std::string line;
    std::getline(file, line);

    while(std::getline(file, line))
    {
        if(line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::stringstream ss(line);
        std::string field;
        Vector<std::string> fields;

        while (std::getline(ss, field, ',')) 
            fields.push_back(field);

        if(fields.size() < 2)
        {
            std::cerr << "Broken file!" << std::endl;
            return Id2Name; 
        }

        try {
            int ID = std::stoi(fields[0]);
            std::string Pinyin = fields[1];

            IDmapInfo info(ID, Pinyin);
            Id2Name.push_back(info);
        } catch (const std::exception &e){
            std::cerr << "Fail to initialize in line! error: " << e.what() << std::endl;
        }
    }

    file.close();
    return Id2Name;
}

void Statistics(const Vector<flightInfo> &f)
{
    int n = f.size();
    if(n==0) return;

    int minDep = 0, maxDep = 0;
    int minDurI = 0, maxDurI = 0;
    int minFareI = 0, maxFareI = 0;
    long minDur = -1, maxDur = -1;

    for(int i=0; i<n; ++i)
    {
        const flightInfo &cur = f[i];

        // 比较出发时间
        if(cur.departureTime < f[minDep].departureTime)
            minDep = i;
        if(cur.departureTime > f[maxDep].departureTime)
            maxDep = i;

        long dur = (long)std::difftime(cur.arrivalTime, cur.departureTime);
        if(minDur < 0 || dur < minDur) 
        { minDur = dur; minDurI = i; }
        if(maxDur < 0 || dur > maxDur)
        { maxDur = dur; maxDurI = i; }

        if(cur.airFares < f[minFareI].airFares) minFareI = i;
        if(cur.airFares > f[maxFareI].airFares) maxFareI = i;
    }

    // 输出
    std::cout << "Statistics of flights:" << std::endl;

    std::string s_minDep = f[minDep].departureTimeStr;
    int id_minDep = f[minDep].flightID;
    std::replace(s_minDep.begin(), s_minDep.end(), '/', '-');
    std::cout << "Earliest departure time: " << s_minDep;
    std::cout << ", Flight ID: " << id_minDep << std::endl;

    std::string s_maxDep = f[maxDep].departureTimeStr;
    int id_maxDep = f[maxDep].flightID;
    std::replace(s_maxDep.begin(), s_maxDep.end(), '/', '-');
    std::cout << "Latest departure time: " << s_maxDep;
    std::cout << ", Flight ID: " << id_maxDep << std::endl;

    double minHours = static_cast<double>(minDur) / 3600.0;
    int id_minDur = f[minDurI].flightID;
    double maxHours = static_cast<double>(maxDur) / 3600.0;
    int id_maxDur = f[maxDurI].flightID;
    std::cout << "Shortest duration: " << std::fixed << std::setprecision(2) 
                                    << minHours << " hours";
    std::cout << ", Flight ID: " << id_minDur << std::endl;
    std::cout << "Longest duration: " << std::fixed << std::setprecision(2) 
                                    << maxHours << " hours";
    std::cout << ", Flight ID: " << id_maxDur << std::endl;

    int minFare = f[minFareI].airFares;
    int id_minFare = f[minFareI].flightID;
    std::cout << "Cheapest flight: ¥" << minFare;
    std::cout << ", Flight ID: " << id_minFare << std::endl;
    int maxFare = f[maxFareI].airFares;
    int id_maxFare = f[maxFareI].flightID;
    std::cout << "Most expensive flight: ¥" << maxFare;
    std::cout << ", Flight ID: " << id_maxFare << std::endl;
}

bool IsNextDay(std::time_t dep, std::time_t arr)
{
    std::tm d = *std::localtime(&dep);
    std::tm a = *std::localtime(&arr);
    if(a.tm_year != d.tm_year) return a.tm_year > d.tm_year;
    if(a.tm_mon != d.tm_mon) return a.tm_mon > d.tm_mon;
    return a.tm_mday > d.tm_mday;
}

inline bool ByFlightID(const flightInfo &a, const flightInfo &b) 
{ return a.flightID < b.flightID; }

void QuickSort(Vector<flightInfo>& v, int lo, int hi,
                bool (*less)(const flightInfo &a, const flightInfo &b))
{
    if(lo >= hi) return;

    int mid = lo + ((hi-lo) >> 1);
    if(less(v[mid], v[lo])) std::swap(v[mid], v[lo]);
    if(less(v[hi], v[lo])) std::swap(v[hi], v[lo]);
    if(less(v[hi], v[mid])) std::swap(v[hi], v[mid]);
    std::swap(v[mid], v[lo]);

    int i = lo+1, j = hi;
    while(i <= j)
    {
        while(i <= j && !less(v[lo], v[i])) ++i;
        while(i <= j && !less(v[j], v[lo])) --j;
        if(i < j) std::swap(v[i], v[j]);
    }

    std::swap(v[lo], v[j]);
    QuickSort(v, lo, j-1, less);
    QuickSort(v, j+1, hi, less);
}

std::string LastSegment(const std::string &p)
{
    std::size_t pos = p.find_last_of('/');
    return (pos == std::string::npos) ? p : p.substr(pos + 1);
}

bool EqualsPos(const std::string &ans, const std::string &query)
{
    if(ans.size() != query.size()) return false;
    for(std::size_t i=0; i<ans.size(); ++i)
        if(std::tolower(static_cast<unsigned char>(ans[i])) !=
            std::tolower(static_cast<unsigned char>(query[i])))
            return false;
    return true;            
}

int FindAirport(const Vector<IDmapInfo> &m, const std::string &input)
{
    for(int i=0; i<m.size(); ++i)
        if(EqualsPos(LastSegment(m[i].Pinyin), input))
            return m[i].ID;
    std::cerr << "Failed to find Airport!" << std::endl;
    return -1;
}

void QueryDirect(const Vector<flightInfo> &flights, int src, int dst)
{
    Vector<flightInfo> cand;
    for(int i=0; i<flights.size(); ++i)
        if(flights[i].departureAirport == src && flights[i].arrivalAirport == dst)
            cand.push_back(flights[i]);

    if(cand.empty())
    { 
        std::cout << "No direct flight." << std::endl;
        return;
    }

    int minFare = cand[0].airFares;
    long minDur = static_cast<long>(std::difftime(cand[0].arrivalTime, cand[0].departureTime));
    for(int i=1; i<cand.size(); ++i)
    {
        long curDur = static_cast<long>(std::difftime(cand[i].arrivalTime, cand[i].departureTime));
        if(cand[i].airFares < minFare) minFare = cand[i].airFares;
        if(curDur < minDur) minDur = curDur;
    }

    QuickSort(cand, 0, cand.size() - 1, ByFlightID);
    
    for(int i=0; i<cand.size(); ++i)
    {
        if(i > 0) std::cout << ", "; 
        std::cout << cand[i].flightID;

        Vector<std::string> tags;
        if(cand[i].airFares == minFare) 
            tags.push_back("cheapest");
        if(static_cast<long>(std::difftime(cand[i].arrivalTime, cand[i].departureTime)) == minDur)
            tags.push_back("shortest duration");
        if(IsNextDay(cand[i].departureTime, cand[i].arrivalTime))
            tags.push_back("+1 day");

        if(!tags.empty())
        {
            std::cout << "(";
            for(int k=0; k<tags.size(); ++k)
            {
                if(k>0) std::cout << ", ";
                std::cout << tags[k];
            }
            std::cout << ")";
        }
    }
    std::cout << std::endl;
}