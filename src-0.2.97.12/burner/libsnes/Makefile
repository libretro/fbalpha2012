
ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
endif
endif

ifeq ($(platform), unix)
   TARGET := libsnes.so
   fpic := -fPIC
   SHARED := -shared
else ifeq ($(platform), osx)
   TARGET := libsnes.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else
   TARGET := snes.dll
   CC = gcc
   CXX = g++
   SHARED := -shared
   LDFLAGS += -static-libgcc -static-libstdc++
endif

all: $(TARGET)

FBA_BURN_DIR := ../../burn
FBA_BURN_DRIVERS_DIR := ../../burn/drivers
FBA_BURNER_DIR := ..
FBA_CPU_DIR := ../../cpu
FBA_LIB_DIR := ../../libs
FBA_INTERFACE_DIR := ../../interface

BURN_BLACKLIST := $(FBA_BURN_DIR)/zet_c68k.cpp $(FBA_BURN_DIR)/sek_c68k.cpp $(FBA_BURNER_DIR)/tracklst.cpp $(FBA_CPU_DIR)/arm7/arm7exec.c $(FBA_CPU_DIR)/arm7/arm7core.c $(FBA_CPU_DIR)/hd6309/6309tbl.c $(FBA_CPU_DIR)/hd6309/6309ops.c $(FBA_CPU_DIR)/konami/konamtbl.c $(FBA_CPU_DIR)/konami/konamops.c $(FBA_CPU_DIR)/m68k/m68k_in.c $(FBA_CPU_DIR)/m6800/6800ops.c $(FBA_CPU_DIR)/m6800/6800tbl.c $(FBA_CPU_DIR)/m6805/6805ops.c $(FBA_CPU_DIR)/m6809/6809ops.c $(FBA_CPU_DIR)/m6809/6809tbl.c $(FBA_CPU_DIR)/sh2/mksh2.cpp $(FBA_CPU_DIR)/sh2/mksh2-x86.cpp $(FBA_CPU_DIR)/m68k/m68kmake.c $(FBA_BURNER_DIR)/wave_writer.cpp $(FBA_CPU_DIR)/m68k/m68kdasm.c $(FBA_LIBSNES_DIR)/menu.cpp $(FBA_CPU_DIR)/sh2/mksh2.cpp $(FBA_BURNER_DIR)/sshot.cpp $(FBA_BURNER_DIR)/conc.cpp $(FBA_BURNER_DIR)/dat.cpp $(FBA_BURNER_DIR)/cong.cpp $(FBA_BURNER_DIR)/image.cpp $(FBA_BURNER_DIR)/misc.cpp

FBA_BURN_DIRS := $(FBA_BURN_DIR) $(FBA_BURN_DIR)/devices $(FBA_BURN_DIR)/sound $(FBA_BURN_DRIVERS_DIR)/atari $(FBA_BURN_DRIVERS_DIR)/capcom $(FBA_BURN_DRIVERS_DIR)/cave $(FBA_BURN_DRIVERS_DIR)/cps3 $(FBA_BURN_DRIVERS_DIR)/galaxian $(FBA_BURN_DRIVERS_DIR)/konami $(FBA_BURN_DRIVERS_DIR)/megadrive $(FBA_BURN_DRIVERS_DIR)/misc $(FBA_BURN_DRIVERS_DIR)/neogeo $(FBA_BURN_DRIVERS_DIR)/pgm $(FBA_BURN_DRIVERS_DIR)/psikyo $(FBA_BURN_DRIVERS_DIR)/sega $(FBA_BURN_DRIVERS_DIR)/taito $(FBA_BURN_DRIVERS_DIR)/toaplan $(FBA_BURN_DRIVERS_DIR)/misc_post90s $(FBA_BURN_DRIVERS_DIR)/misc_pre90s

FBA_CPU_DIRS := $(FBA_CPU_DIR) $(FBA_CPU_DIR)/arm7 $(FBA_CPU_DIR)/arm $(FBA_CPU_DIR)/hd6309 $(FBA_CPU_DIR)/i8039 $(FBA_CPU_DIR)/konami $(FBA_CPU_DIR)/m68k $(FBA_CPU_DIR)/m6502 $(FBA_CPU_DIR)/m6800 $(FBA_CPU_DIR)/m6805 $(FBA_CPU_DIR)/m6809 $(FBA_CPU_DIR)/nec $(FBA_CPU_DIR)/s2650 $(FBA_CPU_DIR)/sh2 $(FBA_CPU_DIR)/z80

FBA_LIB_DIRS := $(FBA_LIB_DIR)/zlib $(FBA_LIB_DIR)/File_Extractor/fex $(FBA_LIB_DIR)/File_Extractor/7z_C $(FBA_LIB_DIR)/ticpp

FBA_SRC_DIRS := $(FBA_BURNER_DIR) $(FBA_BURN_DIRS) $(FBA_CPU_DIRS) $(FBA_BURNER_DIRS) $(FBA_LIB_DIRS)

FBA_CXXSRCS := $(filter-out $(BURN_BLACKLIST),$(foreach dir,$(FBA_SRC_DIRS),$(wildcard $(dir)/*.cpp)))
FBA_CXXSRCS += libsnes.cpp archive.cpp
FBA_CXXOBJ := $(FBA_CXXSRCS:.cpp=.o)
FBA_CSRCS := $(filter-out $(BURN_BLACKLIST),$(foreach dir,$(FBA_SRC_DIRS),$(wildcard $(dir)/*.c)))
FBA_COBJ := $(FBA_CSRCS:.c=.o)

OBJS := $(FBA_COBJ) $(FBA_CXXOBJ)

FBA_DEFINES := -DNDEBUG -DNO_ASMCORE -DNO_AUTOFIRE -DNO_CHEATSEARCH -DNO_COMBO -DUSE_SPEEDHACKS -DNO_PNG -DOLD_AUDIOCORE=0 -D__LIBSNES__ -DLSB_FIRST -DTIXML_USE_TICPP

CFLAGS += -O3 -std=gnu99 $(fpic) -Wno-write-strings $(FBA_DEFINES)
CXXFLAGS += -O3 $(fpic) -Wno-write-strings $(FBA_DEFINES)
LDFLAGS += -Wl,-no-undefined $(fpic)

INCDIRS := -I$(FBA_BURN_DIR) -I$(FBA_BURN_DIR)/sound -I$(FBA_BURN_DIR)/devices -I../../generated -I$(FBA_BURNER_DIR) -I$(FBA_BURNER_DIR)/generated -I$(FBA_CPU_DIR) -I$(FBA_CPU_DIR)/m6809 -I$(FBA_CPU_DIR)/m6805 -I$(FBA_CPU_DIR)/m6800 -I$(FBA_CPU_DIR)/nec -I$(FBA_CPU_DIR)/hd6309 -I$(FBA_CPU_DIR)/arm7 -I$(FBA_CPU_DIR)/i8039 -I$(FBA_CPU_DIR)/konami -I$(FBA_CPU_DIR)/m68k -I$(FBA_CPU_DIR)/m6502 -I$(FBA_CPU_DIR)/z80 -I$(FBA_CPU_DIR)/sh2 -I$(FBA_CPU_DIR)/s2650 -I$(FBA_CPU_DIR)/arm -I$(FBA_BURNER_DIR)/libsnes -I$(FBA_LIB_DIR)/zlib -I$(FBA_LIB_DIR)/File_Extractor -I$(FBA_BURN_DIR)/drivers/capcom -I$(FBA_BURN_DIR)/drivers/cave -I$(FBA_BURN_DIR)/drivers/neogeo -I$(FBA_BURN_DIR)/drivers/psikyo -I$(FBA_BURN_DIR)/drivers/sega -I$(FBA_BURN_DIR)/drivers/toaplan -I$(FBA_BURN_DIR)/drivers/taito -I$(FBA_BURN_DIR)/drivers/misc_post90s -I$(FBA_LIB_DIR)
LIBS := -lz

$(TARGET): $(OBJS)
	@echo "LD $@"
	@$(CXX) -o $@ $(SHARED) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	@echo "CXX $<"
	@$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCDIRS)

%.o: %.c
	@echo "CC $<"
	@$(CC) -c -o $@ $< $(CFLAGS) $(INCDIRS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)

.PHONY: clean
