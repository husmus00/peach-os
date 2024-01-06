#if !defined(IO_H)
#define IO_H

unsigned char insb(unsigned short port);  // ins byte
unsigned short insw(unsigned short port); // ins word

void outb(unsigned short port, unsigned char val);  // out byte
void outw(unsigned short port, unsigned short val); // out word

#endif // IO_H
