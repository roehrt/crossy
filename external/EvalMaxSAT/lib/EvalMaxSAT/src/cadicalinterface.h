#ifndef CADICALINTERFACE_H
#define CADICALINTERFACE_H

#include <cmath>
#include <cassert>
#include <vector>
#include <set>
#include <chrono>

#include "../../../../../../../../../usr/include/complex.h"
#include "cadical.hpp"
#include "external.hpp"

namespace {
    struct Timeout : CaDiCaL::Terminator {
        std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> timeout;
        explicit Timeout(const double timeout) : timeout(std::chrono::system_clock::now() + std::chrono::duration<double>(timeout)) {}
        bool terminate() override {
            return std::chrono::system_clock::now() >= timeout;
        }
    };
}

class Solver_cadical {
    CaDiCaL::Solver *solver;
    CaDiCaL::ExternalPropagator * _propagator = nullptr;
    unsigned int nVar=0;
public:

    Solver_cadical() : solver(new CaDiCaL::Solver()) {
        solver->set("ilb", 0);
        solver->set("ilbassumptions", 0);
        // solver->set("vivify", 0); // TODO
        solver->set("cover", 1);
        solver->set("flush", 1);
        solver->set("restoreflush", 1);

        solver->set("walk", 0);
        solver->set("stabilize", 0);
    }

    ~Solver_cadical() {
        delete solver;
    }

    bool getValue(int lit) {
        return solver->val(lit) > 0;
    }

    std::vector<bool> getSolution() {
        int vars = solver->vars();
        std::vector<bool> res(vars+1);
        for(int i=1; i<=vars; i++)
            res[i] = getValue(i);
        return res;
    }

    unsigned int nVars() {
        return nVar;
    }

    int newVar(bool decisionVar=true) {
         // decisionVar not implemented in Cadical ?
         return ++nVar;
     }

    void addClause(const std::vector<int> &clause) {
        for (int lit : clause) {
            if( abs(lit) > nVar) {
                nVar = abs(lit);
            }
            solver->add(lit);
        }
       solver->add(0);
    }

    void simplify() {
        solver->simplify();
    }

    void connect_external_propagator(CaDiCaL::ExternalPropagator *propagator) {
        _propagator = propagator;
        solver->connect_external_propagator(propagator);
    }

    void disconnect_external_propagator() {
        _propagator = nullptr;
        solver->disconnect_external_propagator();
    }

    void add_observed_var(int var) const {
        solver->add_observed_var(var);
    }

    bool solve(const std::vector<bool>& solution) {
        for(unsigned int i=1; i<solution.size(); i++) {
            if(solution[i]) {
                solver->assume(i);
            } else {
                solver->assume(-(int)i);
            }
        }

        int result = solver->solve();

        if( !( (result == 10) || (result == 20) ) ) {
            return solve(solution);
        }

        return result == 10; // Sat
    }

    bool solve() {
        int result = solver->solve();

        //assert( (result == 10) || (result == 20) ); // Bug? Can happen sometimes...
        if( !( (result == 10) || (result == 20) ) ) {
            return solve();
        }

        return result == 10; // Sat
    }

    template<class T>
    bool solve(const T &assumption) {
        for (int lit : assumption) {
            solver->assume(lit);
        }

        int result = solver->solve();

        //assert( (result == 10) || (result == 20) ); // Bug? Can happen sometimes...
        if( !( (result == 10) || (result == 20) ) ) {
            return solve(assumption);
        }
        return result == 10; // Sat
    }

    template<class T>
    bool solve(const T &assumption, const std::set<int> &forced) {
        for (int lit : forced) {
            solver->assume(lit);
        }
        for (int lit : assumption) {
            if(forced.count(-lit) == 0) {
                solver->assume(lit);
            }
        }

        int result = solver->solve();

        //assert( (result == 10) || (result == 20) ); // Bug? Can happen sometimes...
        if( !( (result == 10) || (result == 20) ) ) {
            return solve(assumption, forced);
        }

        return result == 10; // Sat
    }

    template<class T>
    int solveLimited(const T &assumption, int confBudget, int except=0) {
        for (int lit : assumption) {
            if (lit == except)
                continue;
            solver->assume(lit);
        }

        solver->limit("conflicts", confBudget);

        auto result = solver->solve();

        if(result==10) { // Satisfiable
            return 1;
        }
        if(result==20) { // Unsatisfiable
            return 0;
        }
        if(result==0) { // Limit
            return -1;
        }

        assert(false);
        return 0;
    }


    template<class T>
    int solveWithTimeout(const T &assumption, double timeout_sec, int except=0) {
        solver->reset_assumptions();

        for (int lit : assumption) {
            if (lit == except)
                continue;
            solver->assume(lit);
        }

        solver->connect_terminator(new Timeout(timeout_sec));

        auto result = solver->solve();

        if(result==10) { // Satisfiable
            return 1;
        }
        if(result==20) { // Unsatisfiable
            return 0;
        }
        if(result==0) { // Limit
            return -1;
        }

        assert(false);
        return 0;
    }


    template<class T>
    int solveWithTimeoutAndLimit(const T &assumption, double timeout_sec, int confBudget, int except=0) {
        solver->reset_assumptions();

        for (int lit : assumption) {
            if (lit == except)
                continue;
            solver->assume(lit);
        }

        solver->limit("conflicts", confBudget);
        solver->connect_terminator(new Timeout(timeout_sec));

        auto result = solver->solve();

        if(result==10) { // Satisfiable
            return 1;
        }
        if(result==20) { // Unsatisfiable
            return 0;
        }
        if(result==0) { // Limit
            return -1;
        }

        assert(false);
        return 0;
    }

    template<class T>
    std::vector<int> getConflict(const T &assumptions) {
        std::vector<int> conflicts;
        for (int lit : assumptions) {
            if (solver->failed(lit)) {
                conflicts.push_back(lit);
            }
        }
        return conflicts;
    }

    bool propagate(int assum, std::vector<int> &result) {
        return true;
    }
};



#endif
