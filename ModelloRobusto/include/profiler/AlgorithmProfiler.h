
#ifndef ALGORITHM_PROFILER_H
#define ALGORITHM_PROFILER_H

#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

struct IterationMetrics {
    int iteration = 0;

    double masterSolveTime = 0.0;
    double slaveUpdateTime = 0.0;
    double slaveSolveTime = 0.0;
    double cutGenerationTime = 0.0;
};

struct PerformanceData {
    std::string instanceName;

    double totalTime = 0.0;
    double masterCreationTime = 0.0;
    double slaveCreationTime = 0.0;
    double dataLoadingTime = 0.0;

    int iterations = 0;
    double objectiveValue = 0.0;

    bool converged = false;

    std::vector<IterationMetrics> iterationMetrics;
};

class AlgorithmProfiler {

public:

    using Clock = std::chrono::steady_clock;

    class ScopedTimer {
    private:
        AlgorithmProfiler& profiler;
        std::string name;
        Clock::time_point start;

    public:
        ScopedTimer(AlgorithmProfiler& p, const std::string& n);

        ~ScopedTimer();

        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;
    };

    // Inizializza i dati dell'esecuzione
    explicit AlgorithmProfiler(const std::string& instanceName = "");

    // Registra il tempo di un'operazione generale
    void addTime(const std::string& name, double seconds);

    // Registra le metriche di una singola iterazione
    void addIterationMetrics(int iteration, double masterSolveTime, double slaveUpdateTime, double slaveSolveTime, double cutGenerationTime);

    // Imposta i risultati finali
    void setResults(double objectiveValue, bool converged);

    // Restituisce l'oggetto PerformanceData
    const PerformanceData& getPerformanceData() const;

    // Stampa il report
    void printReport() const;
    // Esporta PerformanceData in CSV
    
    bool saveCSV(const std::string& filename) const;

private:

    PerformanceData data;
};

#endif
