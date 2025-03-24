#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

void process_instruction()
{
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = (instruction >> 21) & 0x7FF;  // Bits <31:21>

    switch (opcode) {
        case 0x6A2:  // HLT
            RUN_BIT = FALSE;  // Detener simulación
            break;

        case 0x558: {  // ADDS Register
            uint32_t Rd = instruction & 0x1F;  // Bits <4:0>
            uint32_t Rn = (instruction >> 5) & 0x1F;  // Bits <9:5>
            uint32_t Rm = (instruction >> 16) & 0x1F;  // Bits <20:16>

            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];  // Manejo de XZR
            int64_t reg_Xm = (Rm == 31) ? 0 : CURRENT_STATE.REGS[Rm];

            int64_t result = reg_Xn + reg_Xm;
            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;  // XZR permanece en 0

            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result < 0) ? 1 : 0;
            break;
        }

        case 0x758: {  // SUBS Register
            uint32_t Rd = instruction & 0x1F;           // Bits <4:0>
            uint32_t Rn = (instruction >> 5) & 0x1F;    // Bits <9:5>
            uint32_t Rm = (instruction >> 16) & 0x1F;   // Bits <20:16>
        
            // Manejar el registro XZR (X31)
            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];
            int64_t reg_Xm = (Rm == 31) ? 0 : CURRENT_STATE.REGS[Rm];
            int64_t result = reg_Xn - reg_Xm;
        
            // Actualizar registro destino
            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;
        
            // Actualizar FLAGS
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;  // Z se activa si resultado es 0
            NEXT_STATE.FLAG_N = (result < 0) ? 1 : 0;  // N se activa si resultado es negativo
            break;
        }
        
        case 0x588: {  // ADDS Immediate
            uint32_t Rd = instruction & 0x1F;
            uint32_t Rn = (instruction >> 5) & 0x1F;
            uint32_t imm12 = (instruction >> 10) & 0xFFF;

            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];
            int64_t imm = imm12;  // No hay shift en este TP
            int64_t result = reg_Xn + imm;

            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;

            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result < 0) ? 1 : 0;
            break;
        }

        case 0x788: {  // SUBS Immediate
            uint32_t Rd = instruction & 0x1F;
            uint32_t Rn = (instruction >> 5) & 0x1F;
            uint32_t imm12 = (instruction >> 10) & 0xFFF;

            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];
            int64_t imm = imm12;  // No hay shift en este TP
            int64_t result = reg_Xn - imm;

            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;

            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result < 0) ? 1 : 0;
            break;
        }

        default:
            printf("Instrucción desconocida: %x\n", opcode);
            break;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;  // Incrementar PC
}