#include <iostream>
#include <vector>


class AirBlock {
    public:
        int id;
        int row;
        int col;

        std::vector<double> traffic;
        std::vector<int> pLevel;
        std::vector<int> gLevel;

        AirBlock(int id, int r, int c): id(id), row(r), col(c) {}
};

class Sector {
    public:
        int id;
        std::vector<AirBlock*> airblocks;

        double capacity;

        Sector(int id): id(id), capacity(0.0) {}
        int getExcess(int t);   //ritorna l'eccesso totale del settore all'intervallo temporale t
    };

class Configuration {
    public:
        int id;

        std::vector<Sector*> sectors;

        Configuration(int id): id(id) {}
        bool checkSimilarity(Configuration&);

        int getExcess(int t);   //ritorna l'eccesso totale della configurazione all'intervallo temporale t
};

class TimeSlot {
public:
    int id;         // indice (0,1,2,...)
    int startHour;  // es. 6
    int endHour;    // es. 7

    TimeSlot(int id, int startHour, int endHour): id(id), startHour(startHour), endHour(endHour) {}
};

class TimeHorizon {
public:
    std::vector<TimeSlot> slots;

    TimeHorizon(int startHour, int endHour) {
        int id = 0;
        for (int h = startHour; h < endHour; ++h) {
            slots.emplace_back(id++, h, h + 1);
        }
    }

    int size() const {
        return slots.size();
    }

    const TimeSlot& get(int t) const {
        return slots[t];
    }
};

class Grid {
public:
    Grid(int rows, int cols);

    int rows() const;
    int cols() const;

    int index(int row, int col) const;
    std::pair<int,int> coordinates(int id) const;

    AirBlock& airBlock(int row, int col);
    AirBlock& airBlock(int id);

    int getNTimeslots();
    int geth();

private:
    int m_rows;
    int m_cols;

    std::vector<AirBlock> m_airBlocks;

    int h;  //minimo slot temporale che deve passare tra due configurazioni diverse
};

class DACData {
    public:
    TimeHorizon time;
    Grid grid;
    std::vector<Sector> sectors;
    std::vector<Configuration> configurations;

    int getNConfigurations();
    std::vector<int> getSimilarConfigurationsIndex(Configuration c);    //restituisce anche lindice di c
};

class CSVReader {
    public:
    DACData readCSV(std::string path);
};

DACData CSVReader::readCSV(std::string path) {
}