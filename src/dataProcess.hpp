#pragma once
#include "Vector.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <filesystem>
#include <ctime>
#include <cstdio>
#include <algorithm>

std::time_t ParseTime(const std::string &t);
std::string FormatHours(long sec);
std::string FormatDateTime(const std::string &raw);

struct flightInfo   // 航班信息，按照csv表头一一对照保存数据
{
    // 能使用int进行保存的一致使用int进行保存
    int flightID;
    int airFares;
    int flightNO;
    int airplaneModel;
    std::string departureTimeStr, arrivalTimeStr;   // 时间字符串便于输出
    std::time_t departureTime, arrivalTime;         // 时间time_t便于比较计算
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
    flightInfo() = default;   // 重加载默认构造函数便于后续文件读取  
};

struct IDmapInfo    // 机场id和机场名映射信息
{
    int ID;
    std::string Pinyin;

    IDmapInfo(int id, std::string name) : ID(id), Pinyin(name) {}
    IDmapInfo() = default;
};

std::time_t ParseTime(const std::string &t)     // 时间解析
{
    std::tm tm = {};    // ctime提供的可用字符串解析的时间数据结构tm
    std::istringstream ss(t);   // 流式输入便于处理
    ss >> std::get_time(&tm, "%m/%d/%Y %H:%M");
    if(ss.fail())   // 捕提取失败信息
        { std::cerr << "Fail to parse time!" << std::endl; return -1; }
    return std::mktime(&tm);    // 转换为time_t数据类型
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
    std::getline(file, line);   // 舍弃表头行

    while(std::getline(file, line))
    {
        if(line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();   //处理csv行末'\r'特殊字符

        std::stringstream ss(line);
        std::string field;
        Vector<std::string> fields;

        while (std::getline(ss, field, ','))   // 按行读取，自动以逗号分隔信息
            fields.push_back(field);

        if(fields.size() < 10)
        {
            std::cerr << "Broken file!" << std::endl;
            return flights; 
        }

        try {
            // stoi将string类型转化为int并自动检查是否越界
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
        } catch (const std::exception &e){   // 捕获初始化失败信息
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

void Statistics(const Vector<flightInfo> &f)   // 整体数据统计处理
{
    int n = f.size();
    if(n==0) return;

    int minDep = 0, maxDep = 0;
    int minDurI = 0, maxDurI = 0;   // 记录最小/最大时间间隔的角标便于访问
    int minFareI = 0, maxFareI = 0;
    long minDur = -1, maxDur = -1;   //记录最小/最大时间间隔本身便于输出

    for(int i=0; i<n; ++i)
    {
        const flightInfo &cur = f[i];

        // 比较出发时间
        if(cur.departureTime < f[minDep].departureTime)
            minDep = i;
        if(cur.departureTime > f[maxDep].departureTime)
            maxDep = i;

        // 比较间隔时间
        long dur = (long)std::difftime(cur.arrivalTime, cur.departureTime);
        if(minDur < 0 || dur < minDur) 
        { minDur = dur; minDurI = i; }
        if(maxDur < 0 || dur > maxDur)
        { maxDur = dur; maxDurI = i; }

        // 比较航班开销
        if(cur.airFares < f[minFareI].airFares) minFareI = i;
        if(cur.airFares > f[maxFareI].airFares) maxFareI = i;
    }

    // 输出
    std::cout << "Statistics of flights:" << std::endl;

    // 输出出发时间极值信息
    int id_minDep = f[minDep].flightID;
    std::cout << "Earliest departure time: " << FormatDateTime(f[minDep].departureTimeStr);
    std::cout << ", Flight ID: " << id_minDep << std::endl;

    int id_maxDep = f[maxDep].flightID;
    std::cout << "Latest departure time: " << FormatDateTime(f[maxDep].departureTimeStr);
    std::cout << ", Flight ID: " << id_maxDep << std::endl;

    // 输出间隔时间极值信息
    int id_minDur = f[minDurI].flightID;
    int id_maxDur = f[maxDurI].flightID;
    std::cout << "Shortest duration: " << FormatHours(minDur) << " hours";
    std::cout << ", Flight ID: " << id_minDur << std::endl;
    std::cout << "Longest duration: " << FormatHours(maxDur) << " hours";
    std::cout << ", Flight ID: " << id_maxDur << std::endl;

    // 输出航班开销极值信息
    int minFare = f[minFareI].airFares;
    int id_minFare = f[minFareI].flightID;
    std::cout << "Cheapest flight: ¥" << minFare;
    std::cout << ", Flight ID: " << id_minFare << std::endl;
    int maxFare = f[maxFareI].airFares;
    int id_maxFare = f[maxFareI].flightID;
    std::cout << "Most expensive flight: ¥" << maxFare;
    std::cout << ", Flight ID: " << id_maxFare << std::endl;
}

// 秒 -> 小时字符串，保留 1~2 位小数（0.5 而不是 0.50，8.92 保持 8.92）
std::string FormatHours(long sec)
{
    if(sec < 0) sec = 0;
    long c = (sec + 18) / 36;                        // 秒 -> 0.01 小时（四舍五入）
    long frac = c % 100;                             // 小数部分，单位 0.01 小时
    if(frac % 10 == 0) frac /= 10;                   // 末尾是 0 只留一位
    else if(frac < 10) return std::to_string(c / 100) + ".0" + std::to_string(frac);
    return std::to_string(c / 100) + "." + std::to_string(frac);
}

// "09/01/2024 00:05" -> "2024-09-01 00:05"
std::string FormatDateTime(const std::string &raw)
{
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%s-%s-%s %s",
                  raw.substr(6, 4).c_str(), raw.substr(0, 2).c_str(),
                  raw.substr(3, 2).c_str(), raw.substr(11, 5).c_str());
    return std::string(buf);
}

bool IsNextDay(std::time_t dep, std::time_t arr)    // 是否隔天判定函数
{
    std::tm d = *std::localtime(&dep);
    std::tm a = *std::localtime(&arr);
    if(a.tm_year != d.tm_year) return a.tm_year > d.tm_year;
    if(a.tm_mon != d.tm_mon) return a.tm_mon > d.tm_mon;
    return a.tm_mday > d.tm_mday;   // 逐年/月/日进行判断
}

inline bool ByFlightID(const flightInfo &a, const flightInfo &b)    // 设计被指函数直接返回大小信息
{ return a.flightID < b.flightID; }

void QuickSort(Vector<flightInfo>& v, int lo, int hi,
                bool (*less)(const flightInfo &a, const flightInfo &b))
{
    // 保证输入区间规范性
    if(lo >= hi) return;

    // 处理lo/mid/hi之间关系，体现排序顺序
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

std::string LastSegment(const std::string &p)   // 规范化拼音只提取最后机场信息
{
    std::size_t pos = p.find_last_of('/');
    return (pos == std::string::npos) ? p : p.substr(pos + 1);
}

bool EqualsPos(const std::string &ans, const std::string &query)    // 判断机场名间是否相等
{
    if(ans.size() != query.size()) return false;
    for(std::size_t i=0; i<ans.size(); ++i)
        if(std::tolower(static_cast<unsigned char>(ans[i])) !=
            std::tolower(static_cast<unsigned char>(query[i])))     // 小写转化
            return false;
    return true;            
}

int FindAirport(const Vector<IDmapInfo> &m, const std::string &input)   // 在维护的线性表伪字典中顺序查找
{
    for(int i=0; i<m.size(); ++i)
        if(EqualsPos(LastSegment(m[i].Pinyin), input))
            return m[i].ID;
    std::cerr << "Failed to find Airport!" << std::endl;
    return -1;
}

void QueryDirect(const Vector<flightInfo> &flights, int src, int dst)   // 核心查找函数
{
    Vector<flightInfo> cand;    // 候选队列
    for(int i=0; i<flights.size(); ++i)
        if(flights[i].departureAirport == src && flights[i].arrivalAirport == dst)
            cand.push_back(flights[i]);

    if(cand.empty())
    { 
        std::cout << "No direct flight." << std::endl;
        return;
    }

    // 统计cheapest/shortest duration/+1 day极值信息
    int minFare = cand[0].airFares;
    long minDur = static_cast<long>(std::difftime(cand[0].arrivalTime, cand[0].departureTime));
    for(int i=1; i<cand.size(); ++i)
    {
        long curDur = static_cast<long>(std::difftime(cand[i].arrivalTime, cand[i].departureTime));
        if(cand[i].airFares < minFare) minFare = cand[i].airFares;
        if(curDur < minDur) minDur = curDur;
    }

    // 按id排序顺序输出
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