#ifndef __SERIAL_H__
#define __SERIAL_H__

void initSerial(void);
void putChar(char);
void putStr(const char *);
void putInt(int);

#define SERIAL_PORT  0x3F8

#endif
