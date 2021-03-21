#ifndef __X86_REG_H__
#define __X86_REG_H__

#include "common.h"

#define PC_START IMAGE_START

enum
{
    R_EAX,
    R_ECX,
    R_EDX,
    R_EBX,
    R_ESP,
    R_EBP,
    R_ESI,
    R_EDI
};
enum
{
    R_AX,
    R_CX,
    R_DX,
    R_BX,
    R_SP,
    R_BP,
    R_SI,
    R_DI
};
enum
{
    R_AL,
    R_CL,
    R_DL,
    R_BL,
    R_AH,
    R_CH,
    R_DH,
    R_BH
};

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */
typedef struct
{
    union {
        union {
            uint32_t _32;
            uint16_t _16;
            uint8_t _8[2];
        } gpr[8];
        struct
        {
            rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
        };
    };
    vaddr_t pc;

} CPU_state;

static inline int check_reg_index(int index)
{
    assert(index >= 0 && index < 8);
    return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
/*TODO finish the reg_b(index) as above, reg_b(index) takes the 8 bits register.
 * Examples: reg_b(0) takes al register, however reg_b(4) takes ah register. al and ah are the lower and upper 8 bits of register ax
 * note that index are range(0, 8)
 * Hint: mapping i and i+4 both to i, mapping i and i+4 to 0 and 1 for lower and upper 8 bits
 */
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[check_reg_index(index) > 3 ? 1 : 0])

static inline const char *reg_name(int index, int width)
{
    extern const char *regsl[];
    extern const char *regsw[];
    extern const char *regsb[];
    assert(index >= 0 && index < 8);

    switch (width)
    {
    case 4:
        return regsl[index];
    case 1:
        return regsb[index];
    case 2:
        return regsw[index];
    default:
        assert(0);
    }
}

void isa_reg_display();
#endif
