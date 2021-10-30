#include "sim.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("no enough arguments.\n");
    } else {
        SIM sim;
        init_sim(&sim);
        sim.load(argv[1]);
        sim.run();
    }
    return 0;
}
