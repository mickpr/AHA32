#include "pwm.h"
//// interrupt, step seconds counter
ISR(TIMER0_COMP_vect) {
	TCNT0=0xFB;
}

void pwm_init(void)
{
	cli();
	// 7 FOC (Force Output Compare 0
	// 3,6 -WGM01, WGM00 - 00 normal, 01 - PWM Phase correct, 10 - CTC, 11 Fast PWM
	// 5,4 - COM01,COM00 - Compare match output mode: 00 - normal (OC0 disconnected), 01 - Toggle OC0 on Compare, 10 - Clear OC0 on compare, 11 - Set OC0 on Compare
	// 2,1,0 - CS02, CS01, CS00 - Clock Select (000 - stop, 001 - CLK, 010 ->CLK/8, 011 - CLK/64, 100 - CLK/256, 101 -CLK/1024, 110 - From T0 (clock on -\_), 111 - From T0 (clock on _/-)
	//TCCRO = ()
	//TCNT0 - rejestr zliczaj¹cy
	//OCR0 - rejestr porównuj¹cy
	// TIMSK - maska przerwañ (OCIE0 - porównanie, TIOE0 - przepe³nienie)

	TCNT0=0xFB;
	OCR0= 0x04;
	TCCR0 = (1<<WGM01) | (1<<COM00) | (1<<CS02) | (1<<CS00);
	TIMSK |= (1<<OCIE0);
	sei();
}
