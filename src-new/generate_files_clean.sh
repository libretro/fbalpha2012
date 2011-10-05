#!/bin/bash

#generate gamelist.txt and generated/driverlist.h
echo ""
echo "clean_driverlist.sh: Remove all pre-generated files from generated/ directory"
echo ""
echo ""
echo "1) Clean all files in the directory 'generated/'"
cd generated
rm *.*
echo ""
echo "2) Remove m68kmake"
cd ..
rm m68kmake
echo ""
echo "3) Remove m68k generated files"
echo ""
rm cpu/m68k/m68kopac.c cpu/m68k/m68kopdm.c cpu/m68k/m68kopnz.c cpu/m68k/m68kops.c cpu/m68k/m68kops.h
echo ""
echo "4) Remove ctvmake"
rm ctvmake
echo "DONE"
