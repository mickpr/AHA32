#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>

#define DQ 6
#define SET_DQ 	DDRD &= ~_BV(DQ)
#define CLR_DQ 	DDRD |= _BV(DQ)
#define IN_DQ 	PIND & _BV(DQ)

void owire_delay(unsigned int __count);

// funkcja inicjalizujaca(zerowanie magistrali)
void owire_reset(void);

// funkcja zapisu bitu na magistrale 1-wire
void owire_write_bit(char wire_bit);

// funkcja odczytu bitu z magistrali 1-wire
char owire_read_bit(void);

// funkcja odczytu bajtu z magistrali 1-wire
unsigned char owire_read_byte(void);

// funkcja zapisu bajtu na magistrale 1-wire
void owire_write_byte(char wire_wartosc);
