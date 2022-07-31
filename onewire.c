#include "onewire.h"

void owire_delay(unsigned int __count)
{
	__asm__ volatile (
		"1: sbiw %0,1" "\n\t"
		"brne 1b"
		: "=w" (__count)
		: "0" (__count)
	);
}

// funkcja inicjalizujaca(zerowanie magistrali)
void owire_reset(void)
{
	CLR_DQ;
	_delay_us(480);  	// opoznienie ok 480us
	SET_DQ;
	_delay_us(480);  	// opoznienie ok 480us
}

// funkcja zapisu bitu na magistrale 1-wire
void owire_write_bit(char wire_bit)
{
	cli(); 				// zablokowanie przerwan
	CLR_DQ;
	_delay_us(10); 	// opoznienie 10us
	if(wire_bit)
		SET_DQ;
	_delay_us(100);	// opoznienie 100us
	SET_DQ;
	sei(); 				// odblokowanie przrerwan
}


// funkcja odczytu bitu z magistrali 1-wire
char owire_read_bit(void)
{
	cli();				// zablokowanie przerwan
	CLR_DQ;
	_delay_us(1);		// opoznienie 1us
	SET_DQ;
	_delay_us(10); 	// opoznienie 10us
	sei();				// odblokowanie pzrerwan
	if(IN_DQ)
		return 1;
	else
		return 0;
}

// funkcja odczytu bajtu z magistrali 1-wire
unsigned char owire_read_byte(void)
{
	unsigned char licznik98;
	unsigned char temp98 = 0;
	for (licznik98=0;licznik98<8;licznik98++)
	{
		if(owire_read_bit())
			temp98|=0x01<<licznik98;
		_delay_us(50);// opoznienie 50us
	}
	return temp98;
}

// funkcja zapisu bajtu na magistrale 1-wire
void owire_write_byte(char wire_wartosc)
{
	unsigned char licznik98;
	unsigned char temp98;
	for (licznik98=0; licznik98<8; licznik98++)
	{
		temp98 = wire_wartosc >> licznik98;
		temp98 &= 0x01;
		owire_write_bit(temp98);
	}
	_delay_us(5);	// opoznienie 5us
}
