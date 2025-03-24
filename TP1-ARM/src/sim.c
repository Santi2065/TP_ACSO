#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = (instruction >> 21) & 0x7FF;

    switch (opcode) {
        case 0x6A2:  // HLT
            RUN_BIT = FALSE;  // Detener simulación
            break;
        default:
            printf("Instrucción desconocida: %x\n", opcode);
            break;
        // Otros casos como ADDS, SUBS, ANDS...
    }
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}
