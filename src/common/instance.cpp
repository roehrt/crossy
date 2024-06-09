#include "instance.hpp"

instance::instance(int n0, int n1) : n0(n0), n1(n1), neighbors(n1), back_neighbors(n0) {}

std::istream &operator>>(std::istream &is, instance &inst) {
    while (is.peek() != 'p')
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string ocr_line;
    std::getline(is, ocr_line);
    std::stringstream ss(ocr_line);
    char p;
    std::string ocr;
    int m;
    ss >> p >> ocr >> inst.n0 >> inst.n1 >> m;
    if (p != 'p' || ocr != "ocr")
        throw std::invalid_argument("Invalid instance format");
    int cutwidth;
    ss >> cutwidth;
    bool has_cutwidth = !ss.fail();
    inst.neighbors.assign(inst.n1, std::vector<int>());
    inst.back_neighbors.assign(inst.n0, std::vector<int>());
    if (has_cutwidth)
        REP(i, 0, inst.n0 + inst.n1) {
            int x;
            is >> x;
        }
    REP(i, 0, m) {
        int u, v;
        is >> u >> v;
        u--;
        v--;
        v -= inst.n0;
        inst.neighbors[v].push_back(u);
        inst.back_neighbors[u].push_back(v);
    }
    REP(i, 0, inst.n0) std::sort(ALL(inst.back_neighbors[i]));
    REP(i, 0, inst.n1) std::sort(ALL(inst.neighbors[i]));
    return is;
}
