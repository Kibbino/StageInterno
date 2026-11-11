
#include "gurobi_c++.h"
#include "include/data/generate_dac_data.h"
#include "include/data/Matrix.h"
#include "include/data/DACCSVGenerator.h"
#include "include/optimization/Master.h"
#include "include/optimization/Slave.h"
#include "include/profiler/AlgorithmProfiler.h"

#include <iostream>
#include <vector>

using namespace std;

int main() {

    try {

        // ==========================================
        // GENERAZIONE DATI
        // ==========================================

        DACCSVGenerator generator(100, 100, 100, 24, 5);
        generator.generateDACCSV("path.csv", 8);

        cout << "Dati DAC generati e salvati in path.csv" << endl;


        // ==========================================
        // INIZIALIZZAZIONE GUROBI
        // ==========================================

        GRBEnv env(true);
        env.set("LogFile", "robusto.log");

        cout << "Inizializzazione ambiente Gurobi..." << endl;

        env.start();


        // ==========================================
        // CARICAMENTO DATI
        // ==========================================

        cout << "Inizio" << endl;

        auto dataStart = AlgorithmProfiler::Clock::now();

        DACData data("path.csv");

        double dataLoadingTime =
            chrono::duration<double>(
                AlgorithmProfiler::Clock::now() - dataStart
            ).count();

        cout << "Lettura dati completata" << endl;


        // ==========================================
        // PROFILER
        // ==========================================

        AlgorithmProfiler profiler("DAC_instance");

        profiler.addTime("DataLoading", dataLoadingTime);


        // ==========================================
        // STAMPA DATI
        // ==========================================

        for (int c = 0; c < data.configurations.size(); c++) {

            cout << "Configurazione " << c << endl;

            for (auto s : data.configurations[c].sectors) {

                cout << " settore "
                     << s->getIndex()
                     << " cap t0="
                     << s->getCapacity(0)
                     << " traffic t0="
                     << s->getTraffic(0)
                     << endl;
            }
        }


        // ==========================================
        // TEMPO TOTALE ALGORITMO
        // ==========================================

        auto totalStart = AlgorithmProfiler::Clock::now();


        // ==========================================
        // CREAZIONE MASTER
        // ==========================================

        Master master = [&]() {

            AlgorithmProfiler::ScopedTimer timer(
                profiler, "MasterCreation"
            );

            return Master(data, env);

        }();


        // ==========================================
        // PRIMA RISOLUZIONE MASTER
        // ==========================================

        MasterSolution sol = master.solve();


        // ==========================================
        // CREAZIONE SLAVE
        // ==========================================

        Slave slave = [&]() {

            AlgorithmProfiler::ScopedTimer timer(
                profiler, "SlaveCreation"
            );

            return Slave(sol, env, data);

        }();


        // ==========================================
        // ALGORITMO PRINCIPALE
        // ==========================================

        bool converged = false;

        while (!converged) {

            cout << "\nIterazione " << master.iter << endl;

            double masterTime = 0.0;
            double slaveUpdateTime = 0.0;
            double slaveSolveTime = 0.0;
            double cutTime = 0.0;


            // --------------------------------------
            // RISOLUZIONE MASTER
            // --------------------------------------

            {
                auto start = AlgorithmProfiler::Clock::now();

                sol = master.solve();

                masterTime =
                    chrono::duration<double>(
                        AlgorithmProfiler::Clock::now() - start
                    ).count();
            }


            // --------------------------------------
            // AGGIORNAMENTO SLAVE
            // --------------------------------------

            {
                auto start = AlgorithmProfiler::Clock::now();

                slave.update(sol);

                slaveUpdateTime =
                    chrono::duration<double>(
                        AlgorithmProfiler::Clock::now() - start
                    ).count();
            }


            // --------------------------------------
            // RISOLUZIONE SLAVE
            // --------------------------------------

            cout << "Risoluzione slave" << endl;

            auto slaveStart = AlgorithmProfiler::Clock::now();

            SlaveSolution slaveSol = slave.solve();

            slaveSolveTime =
                chrono::duration<double>(
                    AlgorithmProfiler::Clock::now() - slaveStart
                ).count();


            // --------------------------------------
            // CONTROLLO CONVERGENZA
            // --------------------------------------

            double max_excess = slaveSol.excess;

            if (max_excess - sol.theta <= data.epsilon) {

                cout << "Aggiustamento completato! "
                     << "Soluzione ottima trovata." << endl;

                converged = true;


                // ----------------------------------
                // STAMPA RISULTATI
                // ----------------------------------

                int T = data.grid.getNTimeslots();
                int C = data.configurations.size();

                if (sol.status == GRB_OPTIMAL) {

                    cout << "\n===== SOLUZIONE OTTIMA =====\n";

                    cout << "Valore obiettivo = "
                         << sol.theta << "\n\n";


                    cout << "\nConfigurazioni attive:\n";

                    for (int t = 0; t < T; t++) {

                        for (int c = 0; c < C; c++) {

                            if (sol.x.at(t, c) > 0.5) {

                                cout << "t = " << t
                                     << " -> configurazione "
                                     << c << '\n';
                            }
                        }
                    }


                    cout << "\nSwitch di configurazione rilevati:\n";

                    for (int t = 0; t < T; t++) {

                        for (int c = 0; c < C; c++) {

                            bool switch_rilevato = false;

                            if (t == 0) {

                                if (sol.x.at(t, c) > 0.5) {
                                    switch_rilevato = true;
                                }

                            } else {

                                if (sol.x.at(t, c) > 0.5 &&
                                    sol.x.at(t - 1, c) < 0.5) {

                                    switch_rilevato = true;
                                }
                            }

                            if (switch_rilevato) {

                                cout << "t = " << t
                                     << " -> Attivata configurazione "
                                     << c;

                                if (sol.s.at(t, c) < 0.5) {

                                    cout << " [ATTENZIONE: "
                                         << "s_c^t non si è attivata "
                                         << "correttamente nel solutore!]";
                                }

                                cout << '\n';
                            }
                        }
                    }

                } else {

                    cout << "Soluzione non ottima. Stato: "
                         << sol.theta << endl;
                }

            } else {

                // ----------------------------------
                // GENERAZIONE CUT
                // ----------------------------------

                auto start = AlgorithmProfiler::Clock::now();

                master.addCut(slaveSol);

                cutTime =
                    chrono::duration<double>(
                        AlgorithmProfiler::Clock::now() - start
                    ).count();
            }


            // --------------------------------------
            // SALVATAGGIO METRICHE ITERAZIONE
            // --------------------------------------

            profiler.addIterationMetrics(
                master.iter,
                masterTime,
                slaveUpdateTime,
                slaveSolveTime,
                cutTime
            );


            // --------------------------------------
            // CONTROLLO ITERAZIONI MASSIME
            // --------------------------------------

            if (master.iter > 100) {

                cout << "Numero massimo di iterazioni raggiunto. "
                     << "Interruzione del processo." << endl;

                break;
            }
        }


        // ==========================================
        // RISULTATI FINALI E PROFILING
        // ==========================================

        double totalElapsedTime =
            chrono::duration<double>(
                AlgorithmProfiler::Clock::now() - totalStart
            ).count();

        profiler.addTime("TotalTime", totalElapsedTime);

        profiler.setResults(
            sol.theta,
            converged
        );


        // ==========================================
        // REPORT E CSV
        // ==========================================

        profiler.printReport();

        bool success = profiler.saveCSV("../Result/profiling.csv");

        if (success) {
            cout << "\nDati di profiling salvati in profiling.csv"
                 << endl;
        } else {
            cout << "\nErrore nel salvataggio dei dati di profiling."
                 << endl;
        }

    } catch (GRBException &e) {

        cout << "Errore Gurobi " << e.getErrorCode()
             << ": " << e.getMessage() << endl;

    } catch (const std::exception& e) {

        cout << "Errore: eccezione: " << e.what() << endl;

    } catch (...) {

        cout << "Errore: eccezione non identificata." << endl;
    }

    return 0;
}
