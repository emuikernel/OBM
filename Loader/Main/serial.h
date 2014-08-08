
#ifndef __SERIAL_H__
#define __SERIAL_H__
// For UART Debug Msg
#if USE_SERIAL_DEBUG
void serial_init(void);
int serial_poll(void);
int serial_read(void);
int serial_write(int c);
void serial_outstr(const char *s);
void serial_outnum(unsigned int n);
#else
#define serial_init()		do{}while(0)
#define serial_poll()		do{}while(0)
#define serial_read()		do{0;}while(0)
#define serial_write(c)		do{}while(0)
#define serial_outstr(s)	do{}while(0)
#define serial_outnum(n)	do{}while(0)
#endif
#endif

