#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "core/ir.h"
#include "core/line.h"
#include "core/error_handling.h"
#include "asm/pass1.h"
#include "asm/pass2.h"
#include "core/symtab.h"


static const char *section_name(Section section) {
    switch (section) {
        case SEC_TEXT: return ".text";
        case SEC_DATA: return ".data";
        default: return "unknown";
    }
}

static void print_image(const char *title, uint32_t base,
                        const uint32_t *words, size_t count) {
    printf("\n%s (%zu words)\n", title, count);
    printf("  Address       Word (hex)\n");
    for (size_t i = 0; i < count; ++i) {
        printf("  0x%08" PRIX32 "    0x%08" PRIX32 "\n",
               base + (uint32_t)(i * 4), words[i]);
    }
}



int main(int argc, char **argv) {
    const char *input_path = (argc > 1) ? argv[1] : INPUT_PROG_PATH;
    app_context *ctx = NULL;
    input_program *input_prog = NULL;
    IR ir = {0};
    Symtab symtab = {0};
    AssemblerOutput output = {0};
    AsmState final_state = {0};
    const AsmConfig cfg = {0x00400000u, 0x10010000u};
    int result = EXIT_FAILURE;
    int pass1_ok = 0;

    ctx = create_app_context(ERROR_LOG_PATH);
    if (!ctx) { fprintf(stderr, "Cannot create application context\n"); goto cleanup; }

    input_prog = create_input_program(ctx, input_path);
    if (!input_prog) { fprintf(stderr, "Cannot open input: %s\n", input_path); goto cleanup; }

    Err e = read_all_lines(ctx, input_prog->input,
                           &input_prog->lines, &input_prog->number_of_line);
    if (e != ERR_OK) { fprintf(stderr, "Input reading failed (error %d)\n", (int)e); goto cleanup; }

    printf("MIPS Two-Pass Assembler\nInput: %s\n", input_path);
    e = assemble_pass1(ctx, &cfg, input_prog->lines,
                       input_prog->number_of_line, &ir, &symtab, &final_state);
    if (e != ERR_OK) { fprintf(stderr, "PASS1 failed (error %d)\n", (int)e); goto cleanup; }
    pass1_ok = 1;

    printf("\nPASS1: OK\n  IR statements: %zu\n  Symbols: %zu\n"
           "  Text size: %" PRIu32 " bytes\n  Data size: %" PRIu32 " bytes\n",
           ir.n, symtab.n, final_state.text_pc, final_state.data_pc);

    printf("\nSYMBOL TABLE\n  Address       Section  Name\n");
    for (size_t i = 0; i < symtab.n; ++i) {
        const Symbol *s = &symtab.v[i];
        printf("  0x%08" PRIX32 "    %-7s  %s\n",
               s->addr, section_name(s->section), s->name);
    }

    e = assemble_pass2(ctx, &cfg, &final_state, &ir, &symtab, &output);
    if (e != ERR_OK) { fprintf(stderr, "PASS2 failed (error %d)\n", (int)e); goto cleanup; }

    printf("\nPASS2: OK\n");
    print_image("TEXT IMAGE", output.text_base, output.text_image, output.text_words);
    print_image("DATA IMAGE", output.data_base, output.data_image, output.data_words);
    printf("\nAssembly completed successfully.\n");
    result = EXIT_SUCCESS;

cleanup:
    assembler_output_free(&output, ctx);
    if (pass1_ok) {
        ir_free(&ir, ctx);
        symtab_free(&symtab, ctx);
    }
    if (input_prog) destroy_input_program(ctx, input_prog);
    if (ctx) destroy_app_context(ctx);
    return result;
}