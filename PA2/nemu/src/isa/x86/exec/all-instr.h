#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(call);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(add);
make_EHelper(sub);
make_EHelper(and);
make_EHelper(xor);
make_EHelper(jmp);
make_EHelper(jcc);
make_EHelper(cmp);
make_EHelper(movzx);
make_EHelper(lea);
make_EHelper(test);
make_EHelper(setcc);
make_EHelper(leave);
make_EHelper(ret);
make_EHelper(nop);
