#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "utils.h"


// STRUTTURA: La grid contiene tutti gli airblock, che ne esistono esattamente solo quelli contenuti in quell'unica grid. 
// I settori conterranno dei puntatori a quegli airblock. Per settare agli airblock il traffico, sarà il costruttore della 
// grid a farlo, o una sua funzione apposita
// prima: creo la griglia, che crea gli airblock
// poi: creo il vettore traffico e lo do alla funzione griglia che setta il traffico agli airblock
// poi: creo i settori che puntano all'id dell'airblock segnato nel csv
// poi: creo le configurazioni, che puntano agli id dei settori indicati nel csv, e controlla che l'unione dei settori formi la griglia
template <class P>
class Matrix {
private:
    int T, C;
    std::vector<P> v;

public:
    Matrix(int T, int C) : T(T), C(C) {
        v.resize(T * C);
    }

    void set(int t, int c, P var) {
        v[t * C + c] = var;
    }

    P& at(int t, int c) {
        return v[t * C + c];
    }

    int getnrows() {
        return T;
    };
    int getncols() {
        return C;
    };

    std::vector<P> getLine(int l) {
        std::vector<P> line;

        for (int c = 0; c < C; c++) {
            line.push_back(v[l * C + c]);
        }

        return line;
    }
};


class AirBlock {
    public:
        int id;
        int row;
        int col;

        std::vector<double> traffic;
        std::vector<int> pLevel;
        std::vector<int> gLevel;

        AirBlock(int id, int r, int c, int ntimes): id(id), row(r), col(c), traffic(ntimes, 0.0) {}

        double getTraffic(int t);
        void setTraffic(int time, double traffic);
};

void AirBlock::setTraffic(int time, double t){
    traffic[time]=t;
}
double AirBlock::getTraffic(int t) {
    return traffic[t];
}

class Sector {
    public:
        int id;
        std::vector<AirBlock*> airblocks;

        std::vector<double> capacity;

        Sector(int id, std::vector<AirBlock*> blocks): id(id), airblocks(blocks) {}
        int getExcess(int t);   //ritorna l'eccesso totale del settore all'intervallo temporale t
        int getid();
        void setCapacity(std::vector<double> cap);
};

void Sector::setCapacity(std::vector<double> cap) {
    capacity=cap;
}

int Sector::getExcess(int t) {
    double total=0.0;
    for(int a=0; a<airblocks.size(); a++) {
        total += airblocks[a]->getTraffic(t);
    }
    return std::max(0.0, total - capacity[t]);
}

int Sector::getid() {
    return id;
}

class Configuration {
    public:
        int id;

        std::vector<Sector*> sectors;

        Configuration(int id, std::vector<Sector*> s): id(id), sectors(s) {}
        bool checkSimilarity(Configuration&);

        int getExcess(int t);   //ritorna l'eccesso totale della configurazione all'intervallo temporale t
};

int Configuration::getExcess(int t) {
    double total=0.0;
    for(int s=0; s<sectors.size(); s++) {
        total+=sectors[s]->getExcess(t);
    }
    return total;
}

bool Configuration::checkSimilarity(Configuration& c2) {
    int intersection=0;
    for(int s1=0; s1<sectors.size(); s1++) {
        for(int s2=0; s2<c2.sectors.size(); s2++) {
            if(sectors[s1]->getid()==c2.sectors[s2]->getid()) {
                intersection++;
            }
        }
    }
    int setUnion = sectors.size() + c2.sectors.size() - intersection;
    if(setUnion>0) {
        double result=static_cast<double>(intersection)/setUnion;
        if(result>=0.50) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
    
}

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
    Grid(int rows, int cols, const TimeHorizon& time, int h);
    Grid(const std::string& filename);

    int getNRows() const;
    int getNCols() const;

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
    TimeHorizon m_time;

    int h;  //minimo slot temporale che deve passare tra due configurazioni diverse
};

int Grid::getNRows() const {
    return m_rows;
}

int Grid::getNCols() const {
    return m_cols;
}



AirBlock& Grid::airBlock(int id)
{
    return m_airBlocks[id];
}

AirBlock& Grid::airBlock(int row, int col)
{
    return m_airBlocks[row * m_cols + col];
}

int Grid::getNTimeslots()
{
    return m_time.size();
}

int Grid::geth()
{
    return h;
}

Grid::Grid(int rows, int cols, const TimeHorizon& time, int h)
    : m_rows(rows), m_cols(cols), m_time(time), h(h)
{
    int id = 0;

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            m_airBlocks.emplace_back(id++, r, c, time.size()); //penso che l'id non serva negli airblock
}

class DACData {
    public:
    // DACData(Grid g, std::vector<Sector> s, std::vector<Configuration> c): grid(g), sectors(s), configurations(c) {}
    DACData(const std::string& filename);
    Grid grid;
    std::vector<Sector> sectors;
    std::vector<Configuration> configurations;

    int getNConfigurations();
    std::vector<int> getSimilarConfigurationsIndex(Configuration& c);    //restituisce anche l'indice di c
    void readRest(const std::string& filename);
    Grid readGrid(const std::string& filename);
    private:

    static std::vector<std::string> split(const std::string& s, char sep);

    static std::string trim(std::string s);
};

Grid DACData::readGrid(const std::string& filename)
{
    std::ifstream in(filename);

    if (!in)
        throw std::runtime_error("Cannot open file.");

    std::string line;
    std::string section;

    TimeHorizon time(0,0);
    time.slots.clear();

    int rows = 0;
    int cols = 0;
    int h = 0;

    while (std::getline(in, line))
    {
        line = trim(line);

        if (line.empty())
            continue;

        if (line[0] == '[')
        {
            section = line;
            std::getline(in, line);   // salta intestazione
            continue;
        }

        if (section == "[TIME]")
        {
            auto f = split(line, ',');

            int id    = std::stoi(f[0]);
            int start = std::stoi(f[1]);
            int end   = std::stoi(f[2]);

            time.slots.emplace_back(id, start, end);
        }
        else if (section == "[GRID]")
        {
            auto f = split(line, ',');

            rows = std::stoi(f[0]);
            cols = std::stoi(f[1]);
            h    = std::stoi(f[2]);
        }
    }

    return Grid(rows, cols, time, h);
}

int DACData::getNConfigurations() {
    return configurations.size();
}

std::vector<int> DACData::getSimilarConfigurationsIndex(Configuration& c1) {
    // algoritmo: rapporto tra settori in comune e insieme dei settori della configurazione c e c'
    std::vector<int> similarIndex;
    for(int c2=0; c2<configurations.size(); c2++) {
        if(c1.checkSimilarity(configurations[c2])) {
            similarIndex.push_back(c2);
        }
    }
    return similarIndex;

}



DACData::DACData(const std::string& filename): grid(readGrid(filename)) {
    readRest(filename);
}
std::string DACData::trim(std::string s)
{
    while (!s.empty() && isspace(s.front()))
        s.erase(s.begin());

    while (!s.empty() && isspace(s.back()))
        s.pop_back();

    if (!s.empty() && s.front() == '"')
        s.erase(s.begin());

    if (!s.empty() && s.back() == '"')
        s.pop_back();

    return s;
}

std::vector<std::string> DACData::split(const std::string& s, char sep)
{
    std::vector<std::string> out;
    std::string token;
    std::stringstream ss(s);
    std::vector<std::vector<double>> capacities;

    while(getline(ss, token, sep))
        out.push_back(trim(token));

    return out;
}


void DACData::readRest(const std::string& filename)  {
    std::ifstream in(filename);

    if(!in)
        throw std::runtime_error("Cannot open file.");

    //--------------------------------------------------
    // Variabili temporanee
    //--------------------------------------------------

    std::string line;
    std::string section;

    Matrix<double>* capPtr=NULL;

    while(getline(in,line))
    {
        line = trim(line);

        if(line.empty())
            continue;

        if(line[0]=='[')
        {
            section = line;
            getline(in,line);     // salta intestazione
            continue;
        }        

        //--------------------------------------------------
        // TRAFFIC
        //--------------------------------------------------

        else if(section=="[TRAFFIC]")
        {
            auto f = split(line,',');

            int block = stoi(f[0]);
            int t     = stoi(f[1]);
            double tr = stod(f[2]);

            AirBlock& b = grid.airBlock(block);

            b.setTraffic(t, tr);
        }

        //--------------------------------------------------
        // SECTORS
        //--------------------------------------------------

        else if(section=="[SECTORS]")
        {
            auto f = split(line, ',');

            int id = stoi(f[0]);

            std::vector<AirBlock*> blocks;

            std::vector<std::string> airIds = split(f[1], ';');

            for (std::string& s : airIds)
            {
                int blockId = stoi(s);

                AirBlock* b = &grid.airBlock(blockId);

                blocks.push_back(b);
            }
            
            sectors.push_back(Sector(id, blocks));
        }


        else if(section=="[CAPACITY]")
        {
        auto f = split(line, ',');

        int sectorId = stoi(f[0]);
        int t = stoi(f[1]);
        double cap = stod(f[2]);
        if(capPtr==NULL) {
            capPtr=new Matrix<double>(sectors.size(), grid.getNTimeslots());
        }
        capPtr->set(sectorId, t, cap);
        }

        //--------------------------------------------------
        // CONFIGURATIONS
        //--------------------------------------------------

        else if(section=="[CONFIGURATIONS]")
        {
            auto f = split(line, ',');

            int id = stoi(f[0]);

            std::vector<Sector*> configSectors;

            std::vector<std::string> secIds = split(f[1], ';');

            for (std::string& s : secIds)
            {
                int sectorId = stoi(s);

                Sector* b = &(sectors[sectorId]);

                configSectors.push_back(b);
            }
            configurations.push_back(Configuration(id, configSectors));
        }
    }
    for (size_t i = 0; i < sectors.size(); ++i)
        {
        sectors[i].setCapacity(capPtr->getLine(i));
        }
}



void generateDACCSV(const std::string& filename, unsigned int seed)
{
    std::mt19937 rng(seed);

    constexpr int ROWS = 20;
    constexpr int COLS = 20;
    constexpr int NTIMES = 15;

    constexpr int NSECTORS = 100;
    constexpr int NCONFIGS = 20;

    std::uniform_int_distribution<int> blockDist(0, ROWS*COLS-1);
    std::uniform_int_distribution<int> sectorSizeDist(8,30);
    std::uniform_real_distribution<double> capacityDist(20.0,50.0);
    std::normal_distribution<double> capacityNoise(1.0,0.05);
    std::normal_distribution<double> noise(1.0, 0.15);

    std::ofstream out(filename);

    //-----------------------------
    // TIME
    //-----------------------------
    out << "[TIME]\n";
    out << "id,start,end\n";

    for(int t=0;t<NTIMES;t++)
        out << t << "," << 6+t << "," << 7+t << "\n";

    out << "\n";

    //-----------------------------
    // GRID
    //-----------------------------
    out << "[GRID]\n";
    out << "rows,cols,h\n";
    out << ROWS << "," << COLS << "," << 2 << "\n\n";

    //-----------------------------
    // SECTORS
    //-----------------------------
    out << "[SECTORS]\n";
    out << "id,airblocks\n";

    std::vector<std::vector<int>> sectorBlocks;

    for(int s=0;s<NSECTORS;s++)
    {
        int size = sectorSizeDist(rng);

        std::set<int> blocks;

        while(blocks.size()<size)
            blocks.insert(blockDist(rng));

        sectorBlocks.emplace_back(blocks.begin(),blocks.end());

        out << s << ",\"";

        for(size_t i=0;i<sectorBlocks.back().size();i++)
        {
            out << sectorBlocks.back()[i];

            if(i+1<sectorBlocks.back().size())
                out << ";";
        }

        out << "\"\n";
    }

    out << "\n";

    out << "\n";

out << "[CAPACITY]\n";
out << "sector,time,capacity\n";

for(int s=0; s<NSECTORS; s++)
{
    double baseCapacity = capacityDist(rng);

    for(int t=0; t<NTIMES; t++)
    {
        double cap = baseCapacity * std::max(0.8, capacityNoise(rng));

        out << s << ","
            << t << ","
            << cap
            << "\n";
    }
}

    

    //-----------------------------
    // TRAFFIC
    //-----------------------------
    out << "[TRAFFIC]\n";
    out << "airblock,time,traffic\n";

    

    for(int k = 0; k < ROWS * COLS; k++)
    {
        // traffico medio caratteristico dell'airblock
        std::uniform_real_distribution<double> baseTraffic(2.0, 8.0);
        double base = baseTraffic(rng);

        for(int t = 0; t < NTIMES; t++)
        {
            double traffic =
                base *
                hourlyFactor(t) *
                std::max(0.2, noise(rng));

            out << k << ","
                << t << ","
                << traffic
                << "\n";
        }
    }

    out << "\n";

    //-----------------------------
    // CONFIGURATIONS
    //-----------------------------
    out << "[CONFIGURATIONS]\n";
    out << "id,sectors\n";

    std::uniform_int_distribution<int> nSectorConfDist(5,12);
    std::uniform_int_distribution<int> sectorIdDist(0,NSECTORS-1);

    std::vector<std::set<int>> configs;

    for(int c=0;c<NCONFIGS;c++)
    {
        int n=nSectorConfDist(rng);

        std::set<int> conf;

        while(conf.size()<n)
            conf.insert(sectorIdDist(rng));

        configs.push_back(conf);

        out << c << ",\"";

        size_t k=0;
        for(int s:conf)
        {
            out << s;
            if(++k<conf.size())
                out << ";";
        }

        out << "\"\n";
    }

    out << "\n";
    

    out.close();

    std::cout << "Istanza salvata in " << filename << std::endl;
}