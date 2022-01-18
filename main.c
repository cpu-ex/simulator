#include "sim.h"
#include <getopt.h>

static struct option optional_args[] = {
    {"data", required_argument, NULL, 'd'},
    {"sld", required_argument, NULL, 's'},
    {"lite", no_argument, NULL, 'l'},
    {"nocache", no_argument, NULL, 'n'},
    {"help", no_argument, NULL, 'h'}
};

static char* HELP_MSG[] = {
    "usage: ./sim CODE [-h] [--data DATA] [--sld SLD] [--lite] [--nocache]\n\n",
    "\trisc-v simulator\n\n",
    "positional arguments:\n",
    "\tCODE\tsupposing the existence of ./bin/code_name.code\n\n",
    "optional arguments:\n",
    "\t-h, --help\tshow this help message and exit\n",
    "\t--data DATA\tprovide a binary data file (relative path supported), default to ./bin/code_name.data\n",
    "\t--sld SLD\tprovide a binary sld file (relative path supported)\n",
    "\t--lite   \tstart simulator in lite mode without gui window and no exception will be checked during excecuting, just looping util end of the program\n",
    "\t--nocache\tstart simulator in nocache mode which is valid in normal gui mode\n"
};

int main(int argc, char* argv[]) {
    // parse arguments
    if (argc < 2) {
        printf("no enough arguments.\n");
        exit(-1);
    }
    int ch, idx;
    int is_lite = 0, is_nocache = 0;
    char code_name[36], data_name[36], sld_name[36];
    // take 1st argument as code name by default
    sprintf(code_name, "bin/%s.code", argv[1]);
    sprintf(data_name, "bin/%s.data", argv[1]);
    while (EOF != (ch = getopt_long(argc, argv, "h", optional_args, &idx))) {
        switch (ch) {
        // data: overwrite default value
        case 'd': sprintf(data_name, "%s", optarg); break;
        // sld
        case 's': sprintf(sld_name, "%s", optarg); break;
        // lite mode flag
        case 'l': is_lite = 1; break;
        // nocache mode flag
        case 'n': is_nocache = 1; break;
        // show help message
        case 'h': { for (int i = 0; i < 10; ++i) printf("%s", HELP_MSG[i]); } return 0;
        default: break;
        }
    }
    // initialize simulator
    static SIM sim;
    init_sim(&sim, is_lite, is_nocache);
    sim.load(&sim, code_name, data_name, sld_name);
    sim.run(&sim);
    return 0;
}
