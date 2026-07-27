#include "gurobi_c++.h"
#include "generate_dac_data.cpp"
#include <iostream>
#include <vector>

using namespace std;



int main() {
    try {
        generateDACCSV("path.csv", 10);
        // Crea l'ambiente
        GRBEnv env(true);
        env.set("LogFile", "nominale.log");
        env.start();

        // Crea il modello
        GRBModel model(env);

        // Variabili
        DACData data("path.csv");
        int T=data.grid.getNTimeslots();
        int C=data.getNConfigurations();
        Matrix<GRBVar> x(T,C);
        for(int t = 0; t < T; t++) {
                for(int c = 0; c < C; c++) {
                    string name = "x_" + to_string(t) + "_" + to_string(c);
                    x.at(t,c)=model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
                }
            }
        Matrix<GRBVar> s(T,C);
        for(int t = 0; t < T; t++) {
                for(int c = 0; c < C; c++) {
                    string name = "s_" + to_string(t) + "_" + to_string(c);
                    s.at(t,c)=model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
                }
            }


        // Vincolo 5.2
        for (int t = 0; t < T; t++) {
            GRBLinExpr sum = 0;
            for (int c = 0; c < C; c++)
                sum += x.at(t,c);

            model.addConstr(sum == 1);
        }
        // Vincolo 5.3
        for (int t = 0; t < T-1; t++) {
            int t_succ=t+1;
            for (int c = 0; c < C; c++) {
                vector<int> c_similar=data.getSimilarConfigurationsIndex(data.configurations[c]);
                GRBLinExpr sum = 0;
                for(int c_similar_idx=0; c_similar_idx < c_similar.size(); c_similar_idx++) {
                    sum += x.at(t_succ, c_similar[c_similar_idx]);
                }

                model.addConstr(x.at(t,c) - sum <= 0);
            }
        }

        // Vincolo 5.4
        for (int t = 0; t < T; t++) {
            int t_prec=t-1;
            for (int c = 0; c < C; c++) {
                if(t==0) {
                    model.addConstr(x.at(t,c) <= s.at(t,c));
                } else {
                    model.addConstr(x.at(t,c) - x.at(t_prec,c) <= s.at(t,c));
                }
            }
        }

        // Vincolo 5.5
        int h=data.grid.geth();
        for(int t=0; t<=T-h; t++) {
            GRBLinExpr sum = 0;
            for(int y=t; y<=t+h-1; y++) {
                    for (int c=0; c<C; c++) {
                        sum += s.at(y,c);
                    }
            }
            model.addConstr(sum<=1);
        }

        // Objective
        GRBLinExpr sum = 0;
        for (int t=0; t<T; t++) {
            for (int c=0; c<C; c++) {
                sum+=x.at(t,c)*data.configurations[c].getExcess(t);
            }
        }
        
        model.setObjective(sum, GRB_MINIMIZE);
        // Ottimizza
        model.optimize();

        // Stampa risultati
        if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {

            cout << "\n===== SOLUZIONE OTTIMA =====\n";
            cout << "Valore obiettivo = " << model.get(GRB_DoubleAttr_ObjVal) << "\n\n";
            cout << "\nConfigurazioni attive:\n";
            for (int t = 0; t < T; t++) {
                for (int c = 0; c < C; c++) {
                    if (x.at(t,c).get(GRB_DoubleAttr_X) > 0.5) {
                        cout << "t = " << t
                            << " -> configurazione " << c << '\n';
                    }
                }
            }

            cout << "\nSwitch di configurazione rilevati:\n";
for (int t = 0; t < T; t++) {
    for (int c = 0; c < C; c++) {
        bool switch_rilevato = false;
        
        if (t == 0) {
            // A t=0 c'è uno switch se la configurazione si attiva per la prima volta
            if (x.at(t, c).get(GRB_DoubleAttr_X) > 0.5) {
                switch_rilevato = true;
            }
        } else {
            // Per t > 0, c'è uno switch se era spenta prima e accesa ora
            if (x.at(t, c).get(GRB_DoubleAttr_X) > 0.5 && x.at(t-1, c).get(GRB_DoubleAttr_X) < 0.5) {
                switch_rilevato = true;
            }
        }

        if (switch_rilevato) {
            cout << "t = " << t << " -> Attivata configurazione " << c;
            // Verifica di controllo (opzionale ma utile per il debugging del modello)
            if (s.at(t, c).get(GRB_DoubleAttr_X) < 0.5) {
                cout << " [ATTENZIONE: s_c^t non si è attivata correttamente nel solutore!]";
            }
            cout << '\n';
        }
    }
}
        } else {

        cout << "Il modello non ha trovato una soluzione ottima.\n";
        cout << "Status = "
            << model.get(GRB_IntAttr_Status)
            << endl;
        }
    } catch (GRBException &e) {
        cout << "Errore Gurobi " << e.getErrorCode()
             << ": " << e.getMessage() << endl;
    } catch (const std::exception& e) {
    cout << "Eccezione: " << e.what() << endl;
}
catch (...) {
    cout << "Eccezione non identificata." << endl;
}
    return 0;
}






// class X {
//     private:
//         int T;
//         int C;
//         std::vector<GRBVar> x;

//     public:
//         X(int T, int C, GRBModel& model) : T(T), C(C) {
//             for(int t = 0; t < T; t++) {
//                 for(int c = 0; c < C; c++) {
//                     string name = "x_" + to_string(t) + "_" + to_string(c);
//                     x.push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name));
//                 }
//             }
            
//         }

//         GRBVar& at(int t, int c) {
//             return x[t * C + c];
//         }
// };

// class S {
//     private:
//         int T;
//         int C;
//         std::vector<GRBVar> s;

//     public:
//         S(int T, int C, GRBModel& model) : T(T), C(C) {
//             for(int t = 0; t < T; t++) {
//                 for(int c = 0; c < C; c++) {
//                     string name = "x_" + to_string(t) + "_" + to_string(c);
//                     s.push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name));
//                 }
//             }
            
//         }

//         GRBVar& at(int t, int c) {
//             return s[t * C + c];
//         }
//     };
// }

// class X {
//     private:
//         int T;
//         int C;
//         std::vector<GRBVar> x;

//     public:
//         X(int T, int C, GRBModel& model) : T(T), C(C) {
//             for(int t = 0; t < T; t++) {
//                 for(int c = 0; c < C; c++) {
//                     string name = "x_" + to_string(t) + "_" + to_string(c);
//                     x.push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name));
//                 }
//             }
            
//         }

//         GRBVar& at(int t, int c) {
//             return x[t * C + c];
//         }
// };

// class S {
//     private:
//         int T;
//         int C;
//         std::vector<GRBVar> s;

//     public:
//         S(int T, int C, GRBModel& model) : T(T), C(C) {
//             for(int t = 0; t < T; t++) {
//                 for(int c = 0; c < C; c++) {
//                     string name = "x_" + to_string(t) + "_" + to_string(c);
//                     s.push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name));
//                 }
//             }
            
//         }

//         GRBVar& at(int t, int c) {
//             return s[t * C + c];
//         }
// };