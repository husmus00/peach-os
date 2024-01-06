#/bin/bash

make clean
./build.sh
qemu-system-x86_64 -hda ./bin/os.bin -display curses