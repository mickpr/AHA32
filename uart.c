#include "uart.h"
#include <string.h>
uint8_t uartbuf[6];
uint8_t rx_pointer =0;

void uart_init(void)
{
   // Set baud rate
   UBRRH = (uint8_t) (USART_UBBR_VALUE >> 8);
   UBRRL = (uint8_t) USART_UBBR_VALUE;

   // Set frame format to 8 data bits, even parity, 1 stop bit
   //UCSRC = (0 << USBS) | (1 << UCSZ1) | (1 << UCSZ0) | (1 << UPM1) | (1 << URSEL);
   UCSRC = (1 << UCSZ1) | (1 << UCSZ0) | (0 << UPM1) | (1 << URSEL);
   // Enable receiver and transmitter and RX interrupt
   UCSRB = ((1 << RXEN) | (1 << TXEN)| (1<<RXCIE));

}

void uart_send(uint8_t u8Data)
{
   // Wait if a byte is being transmitted
   while ((UCSRA & (1 << UDRE)) == 0);

   // Transmit data
   UDR = u8Data;
}

uint8_t uart_receive()
{
   // Wait until a byte has been received
   while ((UCSRA & (1 << RXC)) == 0);
   // Return received data
   return UDR;
}

ISR(USART_RXC_vect)
{
   // uint8_t i;
   char ReceivedByte;
   ReceivedByte = UDR; // Fetch the received byte value into the variable "ByteReceived"
   //UDR = ReceivedByte; // Echo back the received byte back to the computer

   if (((ReceivedByte >='0') && (ReceivedByte<='9')) ||(ReceivedByte=='#')) {
	   uartbuf[0]=uartbuf[1];
   	   uartbuf[1]=uartbuf[2];
   	   uartbuf[2]=uartbuf[3];
   	   uartbuf[3]=uartbuf[4];
   	   uartbuf[4]=ReceivedByte;
   }
   if (ReceivedByte=='#') {
	   uart_printf("Bufor: %s\n\r",uartbuf);
   }
   if (strncmp((char *)uartbuf,"8877#",5)==0) {

	   uart_printf("Correct code\n\r");
	   PORTD |=  (1<<PORTD6);
	   _delay_ms(500);
	   PORTD &= ~(1<<PORTD6); // or 6
   }
}


//Function to transmit a string in Flash
void uart_send_string_from_FLASH(const char* string)
{
  while (pgm_read_byte(&(*string)))
   uart_send(pgm_read_byte(&(*string++)));
}

//Function to transmit a string in RAM
void uart_send_string(unsigned char* string)
{
  while (*string)
	  uart_send(*string++);
}

int uart_printf (const char *fmt, ...)
{
   va_list args;
   //uint8_t i;
   char printbuffer[64];

   va_start (args, fmt);
   // For this to work, printbuffer must be larger than anything we ever want to print.
   //i =
   vsprintf (printbuffer, fmt, args);
   va_end (args);

   // Print the string
   uart_send_string((unsigned char*)printbuffer);
   return 0;
}

/***************************************************
Function to transmit hex format data
first argument indicates type: CHAR, INT or LONG
Second argument is the data to be displayed
***************************************************/
/*

void transmitHex( unsigned char dataType, unsigned long data )
{
unsigned char count, i, temp;
unsigned char dataString[] = "0x        ";

if (dataType == CHAR) count = 2;
if (dataType == INT) count = 4;
if (dataType == LONG) count = 8;

for(i=count; i>0; i--)
{
  temp = data % 16;
  if((temp>=0) && (temp<10)) dataString [i+1] = temp + 0x30;
  else dataString [i+1] = (temp - 10) + 0x41;

  data = data/16;
}

uart_send_string(dataString);
}
*/

