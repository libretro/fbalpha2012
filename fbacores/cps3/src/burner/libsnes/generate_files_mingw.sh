#!/bin/bash

if [ -f ../../generated/driverlist.h ]; then
   echo "Generated files already built, deleting old files first..."
   rm -rf ../../generated
fi

cd ../..
if [ -d ../../generated ]; then
   echo "Directory 'generated' already exists, skipping creation...\n"
else
   mkdir -p generated
fi

#generate gamelist.txt and generated/driverlist.h
echo ""
echo "generate_driverlist.sh: Generate all files from scripts directory"
echo ""
echo "1) gamelist.pl"
perl dep/scripts/gamelist.pl -o generated/driverlist.h -l gamelist.txt burn/drv/cps3
