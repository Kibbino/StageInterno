#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <iostream>

void generateDACCSV(const std::string& filename)
{
    constexpr int ROWS = 20;
    constexpr int COLS = 20;
    constexpr int TIMESLOTS = 15;     

    std::mt19937 rng(42);

    std::uniform_real_distribution<double> trafficDist(0.0,15.0);
    std::uniform_int_distribution<int> pDist(1,3);
    std::uniform_int_distribution<int> gDist(1,3);

    std::ofstream file(filename);

    file << "airblock,time,traffic,pLevel,gLevel\n";

    int id = 0;

    for(int r=0;r<ROWS;r++)
    {
        for(int c=0;c<COLS;c++)
        {
            id = r*COLS+c;

            for(int t=0;t<TIMESLOTS;t++)
            {
                file
                    << id << ","
                    << t << ","
                    << trafficDist(rng) << ","
                    << pDist(rng) << ","
                    << gDist(rng)
                    << "\n";
            }
        }
    }

    file.close();

    std::cout << "CSV generato: " << filename << std::endl;
}