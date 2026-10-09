#pragma once
#include <iostream> 
#include <vector>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <algorithm>


class DACCSVGenerator {
public:
    DACCSVGenerator(int ROWS = 20, int COLS = 20, int NTIMES = 15, int NSECTORS = 100, int NCONFIGS = 20);
    void generateDACCSV(const std::string& filename, unsigned int seed);
    double hourlyFactor(int t);
    
private:
    int ROWS;
    int COLS;
    int NTIMES;
    int NSECTORS;
    int NCONFIGS;
};