#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

void process_instruction()
{
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = (instruction >> 21) & 0x7FF;
    printf("PC: 0x%016lX | Instruction: 0x%08X | Opcode: 0x%X\n", 
       (unsigned long) CURRENT_STATE.PC, instruction, opcode);

    switch (opcode) {
        case 0x6A2:  // HLT
            RUN_BIT = FALSE;  // Detener simulación
            break;

        case 0x558: {  // ADDS Register
            uint32_t Rd = instruction & 0x1F;
            uint32_t Rn = (instruction >> 5) & 0x1F; 
            uint32_t Rm = (instruction >> 16) & 0x1F; 

            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];  // Manejo de XZR
            int64_t reg_Xm = (Rm == 31) ? 0 : CURRENT_STATE.REGS[Rm];

            int64_t result = reg_Xn + reg_Xm;
            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;  // XZR permanece en 0

            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result < 0) ? 1 : 0;
            break;
        }

        case 0x758: {  // SUBS Register
            uint32_t Rd = instruction & 0x1F; 
            uint32_t Rn = (instruction >> 5) & 0x1F;  
            uint32_t Rm = (instruction >> 16) & 0x1F;   
        
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
            int64_t imm = imm12;
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
            int64_t imm = imm12; 
            int64_t result = reg_Xn - imm;

            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;

            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result < 0) ? 1 : 0;
            break;
        }
        case 0x750: {  // ANDS (Shifted Register)
            uint32_t Rd = instruction & 0x1F;
            uint32_t Rn = (instruction >> 5) & 0x1F;
            uint32_t Rm = (instruction >> 16) & 0x1F;

            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];
            int64_t reg_Xm = (Rm == 31) ? 0 : CURRENT_STATE.REGS[Rm];
            int64_t result = reg_Xn & reg_Xm;

            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;

            // Actualizar FLAGS
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;
            NEXT_STATE.FLAG_N = (result < 0) ? 1 : 0;
            break;
        }
        case 0x650: {  // EOR (Shifted Register)
            uint32_t Rd = instruction & 0x1F;
            uint32_t Rn = (instruction >> 5) & 0x1F;
            uint32_t Rm = (instruction >> 16) & 0x1F;

            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];
            int64_t reg_Xm = (Rm == 31) ? 0 : CURRENT_STATE.REGS[Rm];
            int64_t result = reg_Xn ^ reg_Xm;

            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;
            break;
        }
        case 0x550: {  // ORR (Shifted Register)
            uint32_t Rd = instruction & 0x1F;
            uint32_t Rn = (instruction >> 5) & 0x1F;
            uint32_t Rm = (instruction >> 16) & 0x1F;

            int64_t reg_Xn = (Rn == 31) ? 0 : CURRENT_STATE.REGS[Rn];
            int64_t reg_Xm = (Rm == 31) ? 0 : CURRENT_STATE.REGS[Rm];
            int64_t result = reg_Xn | reg_Xm;

            NEXT_STATE.REGS[Rd] = (Rd == 31) ? 0 : result;
            break;
        }
        case 0x0A0: {  // B (Branch)
            int32_t imm26 = (instruction & 0x03FFFFFF);  
            
            // Sign-extend el immediate a 64 bits y multiplicar por 4 (instrucciones de 4 bytes)
            int64_t offset = ((int64_t)((int32_t)(imm26 << 6)) >> 6) * 4;

            NEXT_STATE.PC = CURRENT_STATE.PC + offset;
            break;
        }
        case 0x6B0: {  // BR (Branch Register)
            uint32_t Rn = (instruction >> 5) & 0x1F;

            NEXT_STATE.PC = CURRENT_STATE.REGS[Rn];
            break;
        }
        case 0x2A0: { // B.Cond (Branch Conditional)
            int flag_n = CURRENT_STATE.FLAG_N;
            int flag_z = CURRENT_STATE.FLAG_Z;
            uint8_t cond = (instruction >> 0) & 0xF;  // Condición extraída de los bits [31:28]
            int32_t imm19 = (instruction >> 5) & 0x7FFFF;  // Inmediato de 19 bits

            // Realizamos la extensión de signo de la instrucción inmediata
            imm19 <<= 2;
            if (imm19 & 0x40000) {
                imm19 |= 0xFFF80000;  // Extiende el valor a 32 bits si es negativo
            } else {
                imm19 &= 0x7FFFF;  // Mantén el valor si es positivo
            }

            // Calculamos la nueva dirección de la instrucción
            uint64_t new_address = CURRENT_STATE.PC + (imm19);  

            // Imprimir valores para depuración
            printf("B.Cond | cond: 0x%X | flag_n: %d | flag_z: %d | imm19: 0x%X | new_address: 0x%016lX\n", cond, flag_n, flag_z, imm19, new_address);

            // Evaluamos la condición y realizamos el salto
            switch (cond) {
                case 0x0: // BEQ (Branch if Equal)
                    printf("BEQ condition check: Z flag == 1\n");
                    if (flag_z == 1) {
                        printf("BEQ: Jumping to address 0x%016lX\n", new_address);
                        NEXT_STATE.PC = new_address;
                    } else {
                        printf("BEQ: Not jumping\n");
                    }
                    break;
                case 0x1: // BNE (Branch if Not Equal)
                    printf("BNE condition check: Z flag == 0\n");
                    if (flag_z == 0) {
                        printf("BNE: Jumping to address 0x%016lX\n", new_address);
                        NEXT_STATE.PC = new_address;
                    } else {
                        printf("BNE: Not jumping\n");
                    }
                    break;
                case 0xC: // BGT (Branch if Greater Than)
                    printf("BGT condition check: Z flag == 0 && N flag == 0\n");
                    if (flag_z == 0 && flag_n == 0) {
                        printf("BGT: Jumping to address 0x%016lX\n", new_address);
                        NEXT_STATE.PC = new_address;
                    } else {
                        printf("BGT: Not jumping\n");
                    }
                    break;
                case 0xB: // BLT (Branch if Less Than)
                    printf("BLT condition check: N flag == 1\n");
                    if (flag_n == 1) {
                        printf("BLT: Jumping to address 0x%016lX\n", new_address);
                        NEXT_STATE.PC = new_address;
                    } else {
                        printf("BLT: Not jumping\n");
                    }
                    break;
                case 0xA: // BGE (Branch if Greater or Equal)
                    printf("BGE condition check: N flag == 0\n");
                    if (flag_n == 0) {
                        printf("BGE: Jumping to address 0x%016lX\n", new_address);
                        NEXT_STATE.PC = new_address;
                    } else {
                        printf("BGE: Not jumping\n");
                    }
                    break;
                case 0xD: // BLE (Branch if Less or Equal)
                    printf("BLE condition check: Z flag == 1 || N flag == 1\n");
                    if (flag_z == 1 || flag_n == 1) {
                        printf("BLE: Jumping to address 0x%016lX\n", new_address);
                        NEXT_STATE.PC = new_address;
                    } else {
                        printf("BLE: Not jumping\n");
                    }
                    break;
                default:
                    printf("B.Cond: Unknown condition 0x%X\n", cond);
                    break;
            }
            break;
        }



        
        default:
            printf("Instrucción desconocida: %x\n", opcode);
            break;
    }
    // Para casos donde branchea
    if (opcode != 0x0A0 && opcode != 0x6B0 && opcode != 0x2A0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}
