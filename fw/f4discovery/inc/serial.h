#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#include <stddef.h>

FILE *fopenserial(uint8_t index, uint32_t baudrate, uint8_t *tbuf, size_t tbufsz, uint8_t *rbuf, size_t rbufsz);

struct ringbuf *ser_txbuf(uint8_t index);

#endif // SERIAL2_H_INCLUDED
