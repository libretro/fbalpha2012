
#include "../src/cpu/m68000_intf.cpp"

#ifdef EMU_M68K

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/cpu/m68k/m68kcpu.c"
#include "../src/cpu/m68k/m68kopac.c"
#include "../src/cpu/m68k/m68kopdm.c"
#include "../src/cpu/m68k/m68kopnz.c"
#include "../src/cpu/m68k/m68kops.c"

#ifdef __cplusplus
}
#endif

#endif
