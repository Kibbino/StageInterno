#include "include/profiler/AlgorithmProfiler.h"

AlgorithmProfiler::ScopedTimer::ScopedTimer(AlgorithmProfiler& p, const std::string& n): profiler(p), name(n), start(Clock::now()) {}

AlgorithmProfiler::ScopedTimer::~ScopedTimer() {
    auto end = Clock::now();

    double elapsed =
        std::chrono::duration<double>(
            end - start
        ).count();

    profiler.addTime(name, elapsed);
}

AlgorithmProfiler::AlgorithmProfiler(const std::string& instanceName) {
    data.instanceName = instanceName;
}

void AlgorithmProfiler::addTime(const std::string& name, double seconds) {

    if (name == "MasterCreation") {
        data.masterCreationTime += seconds;
    }
    else if (name == "SlaveCreation") {
        data.slaveCreationTime += seconds;
    }
    else if (name == "DataLoading") {
        data.dataLoadingTime += seconds;
    }
    else if (name == "TotalTime") {
        data.totalTime += seconds;
    }
}

void AlgorithmProfiler::addIterationMetrics(int iteration, double masterSolveTime, double slaveUpdateTime, double slaveSolveTime, double cutGenerationTime) {
    IterationMetrics metrics;

    metrics.iteration = iteration;
    metrics.masterSolveTime = masterSolveTime;
    metrics.slaveUpdateTime = slaveUpdateTime;
    metrics.slaveSolveTime = slaveSolveTime;
    metrics.cutGenerationTime = cutGenerationTime;

    data.iterationMetrics.push_back(metrics);

    data.iterations = static_cast<int>(
        data.iterationMetrics.size()
    );
}

void AlgorithmProfiler::setResults(double objectiveValue, bool converged) {
    data.objectiveValue = objectiveValue;
    data.converged = converged;
}

const PerformanceData& AlgorithmProfiler::getPerformanceData() const {
    return data;
}

void AlgorithmProfiler::printReport() const {

    std::cout << "\n========== PROFILING REPORT ==========\n";

    std::cout << "Instance: " << data.instanceName << "\n";
    std::cout << "Total time: " << data.totalTime << " s\n";
    std::cout << "Master creation: " << data.masterCreationTime << " s\n";
    std::cout << "Slave creation: " << data.slaveCreationTime << " s\n";
    std::cout << "Data loading: " << data.dataLoadingTime << " s\n";
    std::cout << "Iterations: " << data.iterations << "\n";
    std::cout << "Objective value: " << data.objectiveValue << "\n";
    std::cout << "Converged: " << std::boolalpha << data.converged << "\n";

    for (const auto& m : data.iterationMetrics) {
        std::cout << "\nIteration " << m.iteration << "\n";
        std::cout << "Master solve: " << m.masterSolveTime << " s\n";
        std::cout << "Slave update: " << m.slaveUpdateTime << " s\n";
        std::cout << "Slave solve: " << m.slaveSolveTime << " s\n";
        std::cout << "Cut generation: " << m.cutGenerationTime << " s\n";
    }

    std::cout << "======================================\n";
}

bool AlgorithmProfiler::saveCSV(const std::string& filename) const {

    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Errore: impossibile aprire il file per la scrittura risultati: "
                << filename << "\n";
        return false;
    }

    file << std::fixed << std::setprecision(6);

    // ==========================================
    // INFORMAZIONI GENERALI DELL'ISTANZA
    // ==========================================

    file << "InstanceName,TotalTime,MasterCreationTime,"
        << "SlaveCreationTime,DataLoadingTime,"
        << "Iterations,ObjectiveValue,Converged\n";

    file << data.instanceName << ","
        << data.totalTime << ","
        << data.masterCreationTime << ","
        << data.slaveCreationTime << ","
        << data.dataLoadingTime << ","
        << data.iterations << ","
        << data.objectiveValue << ","
        << data.converged << "\n";


    // ==========================================
    // METRICHE DELLE SINGOLE ITERAZIONI
    // ==========================================

    file << "\n";

    file << "Iteration,MasterSolveTime,SlaveUpdateTime,"
        << "SlaveSolveTime,CutGenerationTime\n";

    for (const auto& m : data.iterationMetrics) {

        file << m.iteration << ","
            << m.masterSolveTime << ","
            << m.slaveUpdateTime << ","
            << m.slaveSolveTime << ","
            << m.cutGenerationTime
            << "\n";
    }

    file.close();
    return true;
}

