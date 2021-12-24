#include "sim.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("no enough arguments.\n");
    } else {
        SIM sim;
        init_sim(&sim);
        sim.load(&sim, argv[1], (argc > 2) ? argv[2] : NULL);
        sim.run(&sim);
    }
    return 0;
}
