#include "../src/global.h"
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
            u32 input;
            char output[36];
            // calculate file size
            fseek(code, 0, SEEK_END);
            u64 size = ftell(code) >> 2;
            fseek(code, 0, SEEK_SET);
            // disasm
            for (int i = 0; i < size; i++) {
                // fetch
                u32 input;
                fread(&input, 1, 4, code);
                format2big(input);
                // decode
                INSTR instr = { .raw = input };
                disasm(instr, output);
                printf("0x%08X : %08X\t%s\n", 0x100 + (i << 2), input, output);
                input = 0;
                memset(output, 0, 36);
            }
            fclose(code);
        }
    }
    return 0;
}
