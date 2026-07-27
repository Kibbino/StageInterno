
#include "gurobi_c++.h"
#include "generate_dac_data.h"
#include "Matrix.h"
#include "utils.h"
#include "Master.h"
#include "Slave.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
    try {
        generateDACCSV("path.csv", 10);
        // Crea l'ambiente
        GRBEnv env(true);
        env.set("LogFile", "robusto.log");
        env.start();

        // Variabili
        DACData data("path.csv");
        Master master(data, env);

        // for(int)
        // GRBLinExpr sum = 0;
        // for (int t=0; t<T; t++) {
        //     for (int c=0; c<C; c++) {
        //         sum+=x.at(t,c)*data.configurations[c].getExcess(t);
        //     }
        // }

        // Objective
        

        bool converged=false;
        int iter=1;
        while(!converged) {
            MasterSolution sol=master.solve();
            Slave slave(sol, env, data);
            SlaveSolution slaveSol=slave.solve();
            double max_excess=slaveSol.excess;
            if (max_excess - sol.theta <= data.epsilon) {
                cout << "Aggiustamento completato! Soluzione ottima trovata." << endl;
                converged = true;
            } else {
                master.addCut(slaveSol);                     
            }
            if (master.iter > 100) {
                cout << "Numero massimo di iterazioni raggiunto. Interruzione del processo." << endl;
                break;
            }
        }

    } catch (GRBException &e) {
        cout << "Errore Gurobi " << e.getErrorCode()
             << ": " << e.getMessage() << endl;
    } catch (const std::exception& e) {
    cout << "Eccezione: " << e.what() << endl;
} catch (...) {
    cout << "Eccezione non identificata." << endl;
}
    return 0;
}
