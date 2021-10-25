#include "../src/types.h"
#include "../src/instr.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("no enough arguments.\n");
    } else {
        char code_name[36];
        sprintf(code_name, "./bin/%s.code", argv[1]);
        FILE* code = fopen(code_name, "r");
        if (code == NULL) {
            printf("invalid file name: %s.\n", argv[1]);
            exit(-1);
        } else {
            char input[33];
            char output[36];
            while (!feof(code)) {
                fgets(input, 34, code);
                input[32] = '\0';
                if (strlen(input) == 32) {
                    INSTR instr = { .raw = str2int(input) };
                    disasm(instr, output);
                    printf("%s\t%s\n", input, output);
                }
                memset(input, 0, 32);
                memset(output, 0, 36);
            }
            fclose(code);
        }
    }
    return 0;
}
