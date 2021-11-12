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
            u32 input;
            char output[36];
            // calculate file size
            fseek(code, 0, SEEK_END);
            u64 size = ftell(code);
            fseek(code, 0, SEEK_SET);
            // disasm
            for (int i = 0; i < size; i++) {
                u8 byte;
                fread(&byte, 1, 1, code);
                input <<= 8;
                input |= byte;
                if (i % 4 == 3) {
                    INSTR instr = { .raw = input };
                    disasm(instr, output);
                    printf("0x%08X : %08X\t%s\n", i / 4 * 4, input, output);
                    input = 0;
                    memset(output, 0, 36);
                }
            }
            fclose(code);
        }
    }
    return 0;
}
