add-symbol-file ../build/kernelfull.o 0x100000
# add-symbol-file ../programs/blank/blank.elf 0x400000
add-symbol-file ../programs/shell/shell.elf 0x400000
target remote | qemu-system-i386 -hda ./os.bin -S -gdb stdio


