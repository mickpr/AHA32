#include "timer.h"

volatile uint8_t sec,min,hour; // softwareowy zegar systemowy

// interrupt, step seconds counter
ISR(TIMER1_COMPA_vect) {
	TCNT1H=0xD5;
	TCNT1L=0xD0;
	//uart_printf("%02d:%02d:%02d\r",hour,min,sec);

	// odliczaj czas
	sec++;
	if (sec>59) {
		sec=0;
		min++;
	}
	if (min>59) {
		min=0;
		hour++;
	}
	if (hour>23) {
		hour=0;
	}
}

void timer_init(void)
{
	cli();
	TCNT1H=0xD5;
	TCNT1L=0xD0;
	//TCCR1A
	//7,6 - funkcjonowanie wyjsc OC1A - tutaj zwykle piny GPIO (0,0)
	//5,4 - funkcjonowanie wyjsc OC1B - tutaj zwykle piny GPIO (0,0)
	// 3  - force Output compare A
	// 2  - force output compare B
	//1,0 - tryb. CTC (0,0) (WGM11=0, WGM10=0)
	//TCCR1B
	// 7,6 - funkcje Input capture noise itd... nie uzywamy
	// 5 - zarezerwowany (0)
	// 4,3 - funkcja licznika - CTC zlicza od zera do OCR1A -> (0,1) -> (WGM13=0, WGM12=1,)
	// 2,0 - clock select -> (1,0,1) =fclk/1024
	TCCR1A = 0;
	TCCR1B=(1<<WGM12)|(1<<CS12)|(1<<CS10); // crystal clock/1024

	OCR1AH=0x2A; // 1 sec = 2A2F, wiec tutaj: 2A
	OCR1AH=0x2E; // 2F-1 = 2E

    TIMSK |= (1 << OCIE1A); // przerwanie przepelnienia
    sei();
}
