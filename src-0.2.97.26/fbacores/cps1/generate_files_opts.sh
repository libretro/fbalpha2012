#!/bin/bash

if [ -f src/dep/generated/driverlist.h ]; then
   rm -rf src/dep/generated
fi

if [ -d src/dep/generated ]; then
   echo 'Directory 'src/dep/generated' already exists, skipping creation...'
else
   mkdir src/dep/generated
fi

#generate gamelist.txt and src/dep/generated/driverlist.h
perl src/dep/scripts/gamelist.pl -o src/dep/generated/driverlist.h -l gamelist.txt src/burn/drv/capcom

#compile m68kmakeecho 
gcc -o m68kmake src/cpu/m68k/m68kmake.c

#create m68kops.h with m68kmake
./m68kmake src/cpu/m68k/ src/cpu/m68k/m68k_in.c

g++ -D__LIBRETRO_OPTIMIZATIONS__ -o ctvmake src/burn/drv/capcom/ctv_make.cpp
./ctvmake > src/dep/generated/ctv.h
