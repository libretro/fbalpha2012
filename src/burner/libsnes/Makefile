TARGET = libsnes.so

all: $(TARGET)

FBA_BURN_DIR := ../../burn
FBA_BURNER_DIR := ../
FBA_CPU_DIR := ../../cpu
FBA_LIB_DIR := ../../lib
FBA_INTERFACE_DIR := ../../interface

FBA_SRC_DIRS := $(FBA_BURN_DIR) $(FBA_CPU_DIRS) $(FBA_BURNER_DIR)

FBA_BURN_DIRS := $(FBA_BURN_DIR) $(FBA_BURN_DIR)/atari $(FBA_BURN_DIR)/capcom $(FBA_BURN_DIR)/cave $(FBA_BURN_DIR)/cps3 $(FBA_BURN_DIR)/galaxian $(FBA_BURN_DIR)/konami $(FBA_BURN_DIR)/megadrive $(FBA_BURN_DIR)/misc $(FBA_BURN_DIR)/neogeo $(FBA_BURN_DIR)/pgm $(FBA_BURN_DIR)/psikyo $(FBA_BURN_DIR)/sega $(FBA_BURN_DIR)/snes $(FBA_BURN_DIR)/taito $(FBA_BURN_DIR)/toaplan $(FBA_BURN_DIR)/misc/post90s $(FBA_BURN_DIR)/misc/pre90s

FBA_CPU_DIRS := $(FBA_CPU_DIR)/arm7 $(FBA_CPU_DIR)/hd6309 $(FBA_CPU_DIR)/i8039 $(FBA_CPU_DIR)/konami $(FBA_CPU_DIR)/m68k $(FBA_CPU_DIR)/m6502 $(FBA_CPU_DIR)/m6800 $(FBA_CPU_DIR)/m6805 $(FBA_CPU_DIR)/m6809 $(FBA_CPU_DIR)/nec $(FBA_CPU_DIR)/s2650 $(FBA_CPU_DIR)/sh2 $(FBA_CPU_DIR)/z80

FBA_LIB_DIRS := $(FBA_LIB_DIR)/zlib $(FBA_LIB_DIR)/File_Extractor

FBA_BURNER_DIRS := $(FBA_BURNER_DIR) libsnes.cpp

FBA_CXXSRCS := $(foreach dir,$(FBA_SRC_DIRS),$(wildcard $(dir)/*.cpp))
FBA_CXXOBJ := $(FBA_CXXSRCS:.cpp=.o)
FBA_CSRCS := $(foreach dir,$(FBA_SRC_DIRS),$(wildcard $(dir)/*.c))
FBA_COBJ := $(FBA_CSRCS:.c=.o)

OBJS := $(FBA_COBJ) $(FBA_CXXOBJ) libsnes.o

FBA_DEFINES := -DNDEBUG -DNO_ASMCORE -DNO_AUTOFIRE -DNO_CHEATSEARCH -DNO_COMBO -DUSE_SPEEDHACKS -DNO_PNG -DOLD_AUDIOCORE=0 -D__LIBSNES__

CFLAGS += -O3 -g -std=gnu99 -fPIC $(FBA_DEFINES)
CXXFLAGS += -O3 -g -fPIC $(FBA_DEFINES)
LDFLAGS += -Wl,-no-undefined

INCDIRS := -I$(FBA_BURN_DIR) -I$(FBA_BURNER_DIR) -I$(FBA_BURNER_DIR)/generated -I$(FBA_CPU_DIR)/m6809 -I$(FBA_CPU_DIR)/m6805 -I$(FBA_CPU_DIR)/m6800 -I$(FBA_CPU_DIR)/nec -I$(FBA_CPU_DIR)/hd6309 -I$(FBA_CPU_DIR)/arm7 -I$(FBA_CPU_DIR)/i8039 -I$(FBA_CPU_DIR)/konami -I$(FBA_CPU_DIR)/m68k -I$(FBA_CPU_DIR)/m6502 -I$(FBA_CPU_DIR)/z80 -I$(FBA_CPU_DIR)/sh2 -I$(FBA_CPU_DIR)/s2650 -I$(FBA_BURNER_DIR)/libsnes -I$(FBA_LIB_DIR)/zlib -I$(FBA_LIB_DIR)/File_Extractor -I$(FBA_BURN_DIR)/capcom -I$(FBA_BURN_DIR)/cave -I$(FBA_BURN_DIR)/neogeo -I$(FBA_BURN_DIR)/psikyo -I$(FBA_BURN_DIR)/sega -I$(FBA_BURN_DIR)/toaplan
LIBS := -lz

$(TARGET): $(OBJS)
	$(CXX) -o $@ -shared $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCDIRS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCDIRS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)

.PHONY: clean
