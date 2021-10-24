#include "sim.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("no enough arguments.\n");
    } else {
        SIM sim;
        init_sim(&sim, 0x10000);
        sim.load(argv[1]);
        sim.core->store(0x10100, 1, 2);
        sim.core->store(0x10104, 2, 2);
        sim.run();
    }
    return 0;
}
