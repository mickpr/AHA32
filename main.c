#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <util/twi.h>
#include "uart.h"
//#include "i2c.h"
//#include "spi.h"
#include "onewire.h"

#include "ip_config.h"
#include <inttypes.h>
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "avr_compat.h"
#include "net.h"
#include "timer.h"
#include "pwm.h"
#include "timeconv.h"
#include "webpages.h"

// definiowanie statycznej tablicy string we flash
#define P(name)   static const prog_uchar name[] PROGMEM  // declare a static string

// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mymac[6] = {0x54,0x55,0x58,0x10,0x00,0x24};
// how did I get the mac addr? Translate the first 3 numbers into ascii is: TUX
//static uint8_t myip[4] = {192,168,0,100};
//static uint8_t gwip[4] = {192,168,0,1};
static uint8_t myip[4] = {192,168,39,124};
static uint8_t gwip[4] = {192,168,39,1};
uint8_t haveGWmac=0;
//extern uint8_t gwmacaddr[6];
static uint8_t authorized_client_ip[4];

// listen port for tcp/www (max range 1-254)
#define MYWWWPORT 80
// listen port for udp
#define MYUDPPORT 1234
#define MYTCPPORT 1600
//--------------------------------------------------------
extern volatile uint8_t sec,min,hour; // softwareowy zegar systemowy (w timer.c)
//--------------------------------------------------------
#define BUFFER_SIZE 1460
static uint8_t buf[BUFFER_SIZE+1];
#define COMMAND_BUFFER_SIZE 25
//static uint8_t command[COMMAND_BUFFER_SIZE];
char * p_command; // pointer to tokenized command
//0123456789012345678901234<-25
//SWITCH SW1

//NTP client------------------------------------------------------------
// this is were we keep time (in unix gmtime format):
// Note: this value may jump a few seconds when a new ntp answer comes.
// You need to keep this in mid if you build an alarm clock. Do not match
// on "==" use ">=" and set a state that indicates if you already triggered the alarm.
static uint32_t time=0;
#define NTPCLIENTPORT 2721
//static uint8_t ntpip[4] = {212,244,36,227};//tempus1.gum.gov.pl
static uint8_t ntpip[4] = {212,244,36,228};//tempus2.gum.gov.pl
//static uint8_t ntpip[4] = {80,50,231,226};//ntp1.tp.pl
//static uint8_t ntpip[4] = {217,96,29,26}; //ntp2.tp.pl
enum {IDLE, NEED_REQUEST, REQUEST_SEND, RESPONSE_RECEIVED} NTPState = IDLE;
int8_t hours_offset_to_utc=20;
//----------------------------------------------------------------------

// the password string (only the first 5 char checked), (only a-z,0-9,_ characters):
static char password[]="sw"; // must not be longer than 9 char

char temperature[]="xxxxxx\0"; // temperature string


//========================================================
char *odczyt_temp() {
// odczytanie temperatury na czujniku
  unsigned char lsb = 0;         // mlodsze bity odczytane z czujnika temperatury
  unsigned char msb = 0;         // starsze bity odczytane z czujnika temperatury
  float temp;
//  int k;
  char cStringBuffer[8];
//  LCD_Initalize();
//  LCD_WriteText("AVR TERMOMETR");
//  LCD_Clear();

  for(;;){
  owire_reset();               	// sekwencja inicjalizujaca
  owire_write_byte(0xCC);       // rozkaz pomin ROM
  owire_write_byte(0x44);       // rozkaz konwertuj temperature
  _delay_ms(750);
  owire_reset();
  owire_write_byte(0xCC);       // rozkaz pomin ROM
  owire_write_byte(0xBE);       // rozkaz umozliwiajacy odczytanie temperatury
  lsb = owire_read_byte();      // odczytanie mlodszych bitów temperatury
  msb = owire_read_byte();      // odczytanie starszych bitów temperatury


  temp=(float)(lsb+(msb<<8))/16;

  dtostrf(temp,1,1,cStringBuffer);
  //   LCD_WriteText(cStringBuffer);
  //   LCD_GoTo(0,0);
  }
	//strcpy(temperature,cStringBuffer);
  strcpy(temperature,"35\n");
	return temperature;
}

// takes a string of the form password/commandNumber and analyse it
// return values: -1 invalid password, otherwise command number
//                -2 no command given but password valid
//                -3 valid password, no command and no trailing "/"
int8_t analyse_get_url(char *str)
{
    uint8_t loop=1;
    uint8_t i=0;
    while(loop){
        if(password[i]){
            if(*str==password[i]){
                str++;
                i++;
            }else{
            return(-1); }
        }else{
            // end of password
            loop=0;
        }
    }
    // is is now one char after the password
    if (*str == '/'){
        str++;
    }else{
        return(-3);
    }
    // check the first char, garbage after this is ignored (including a slash)
    if (*str < 0x3a && *str > 0x2f){
        // is a ASCII number, return it
        return(*str-0x30);
    }
    return(-2);
}

// answer HTTP/1.0 301 Moved Permanently\r\nLocation: password/\r\n\r\n
// to redirect to the url ending in a slash
uint16_t moved_perm(uint8_t *buf)
{
        uint16_t plen;
        plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 301 Moved Permanently\r\nLocation: "));
        plen=fill_tcp_data(buf,plen,password);
        plen=fill_tcp_data_p(buf,plen,PSTR("/\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<h1>301 Moved Permanently</h1>\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("add a trailing slash to the url\n"));
        return(plen);
}

uint16_t http200ok(void)
{
        return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n")));
}


uint16_t http401unauthorized(void)
{
	return(fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized access.</h1>")));
}



// prepare the web page by writing the data to the TCP send buffer
uint16_t print_webpage(uint8_t *buf,uint8_t on_off)
{
        uint16_t plen;
        char clock[12];

        plen=http200ok();
        plen=fill_tcp_data_p(buf,plen,PSTR("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" /></head><body><center>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<script>document.write(\"<table height=35%><tr><td></td></tr></table>\");</script>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divhead\">Device 1</div>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divbody\" style=\"height:250;\" >"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<br><A href=\"../menu.htm\">MENU</a><p><br>\n"));
        plen=fill_tcp_data_p(buf,plen,PSTR("<table width=98% border=1px><tr><td>Time:</td>"));

        if (time!=0) {
        	plen=fill_tcp_data_p(buf,plen,PSTR("<td>"));
        	sprintf_P(clock,PSTR("%02u:%02u:%02u"),hour,min,sec);
        	plen=fill_tcp_data(buf,plen,clock);
        } else {
        	plen=fill_tcp_data_p(buf,plen,PSTR("<td bgcolor=#FF4040>Not sync w/NTP."));
        }
        plen=fill_tcp_data_p(buf,plen,PSTR("</td></tr>"));

        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Temperature #1:</td><td bgcolor=#FF4040>"));
        //56
		plen=fill_tcp_data_p(buf,plen,temperature);
		plen=fill_tcp_data_p(buf,plen,PSTR("&deg;</td></tr>"));
        switch (on_off) {
        	case 0:
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td><a href=\"./2\">OFF</a></td></tr>"));
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td><a href=\"./1\">OFF</a></td></tr>"));
        		break;
        	case 1:
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td><a href=\"./3\">OFF</a></td></tr>"));
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td bgcolor=#FF4040 color=white><a href=\"./0\">ON</a></td></tr>"));
        		break;
        	case 2:
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td bgcolor=#FF4040 color=white><a href=\"./0\">ON</a></td></tr>"));
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td><a href=\"./3\">OFF</a></td></tr>"));
        		break;
        	case 3:
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td bgcolor=#FF4040 color=white><a href=\"./1\">ON</a></td></tr>"));
                plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td bgcolor=#FF4040 color=white><a href=\"./2\">ON</a></td></tr>"));
        		break;
        	default:
        		// page call without parameters
        		if (PORTD & (1<<PORTD6)) {
        			if (PORTD & (1<<PORTD7)) {
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td bgcolor=#FF4040 color=white><a href=\"./1\">ON</a></td></tr>"));
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td bgcolor=#FF4040 color=white><a href=\"./2\">ON</a></td></tr>"));
        			}
        			else {
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td bgcolor=#FF4040 color=white><a href=\"./0\">ON</a></td></tr>"));
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td><a href=\"./3\">OFF</a></td></tr>"));
        			}
       			}
        		else {
        			if (PORTD & (1<<PORTD7)) {
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td><a href=\"./3\">OFF</a></td></tr>"));
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td bgcolor=#FF4040 color=white><a href=\"./0\">ON</a></td></tr>"));
        			}
        			else {
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik1:</td><td><a href=\"./2\">OFF</a></td></tr>"));
                        plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Przekaznik2:</td><td><a href=\"./1\">OFF</a></td></tr>"));
        			}
        		}
        		break;
        }
        plen=fill_tcp_data_p(buf,plen,PSTR("</table>"));
        plen=fill_tcp_data_p(buf,plen,PSTR("</div></center></body></html>"));
        return(plen);
}

void delay_ms(unsigned int ms) /* delay for a minimum of <ms> */
{
        // we use a calibrated macro. This is more
        // accurate and not so much compiler dependent
        // as self made code.
        while(ms){
                _delay_ms(0.96);
                ms--;
        }
}

#if ENABLE_UDP_RESPONSER
void decode_udp_command(uint8_t * source, uint8_t * dest, int len) {
	uint8_t i=0;
	if (len<COMMAND_BUFFER_SIZE) {
		while(i<len){
		   dest[i]=source[i];
		   i++;
		}
		dest[i]='\0';
	}
	else {
		strcpy((char *)dest,"error\0");
	}

	p_command = strtok((char *)dest," "); // tokenize
	if (strcmp(p_command,"SW")==0)
		uart_printf("we 've got command: %s\n",p_command);

}
#endif

void decode_execute_and_response_tcp_command(char *cmd) {
	char * pcmd; // pointer
	pcmd = strtok(cmd," -,.");
	if (strcmp("GET",pcmd)==0) {
		pcmd = strtok(NULL," -,.");
		// GET SW1
		if (strcmp("SW1",pcmd)==0) {
			if (PORTD & (1<<PORTD7)) {
				strcpy(cmd,"ON\0");
			} else {
				strcpy(cmd,"OFF\0");
			}
			return;
		}
		// GET SW2
		if (strcmp("SW2",pcmd)==0) {
			if (PORTD & (1<<PORTD6)) {
				strcpy(cmd,"ON");
			} else  {
				strcpy(cmd,"OFF");
			}
			return;
		}
	}
	// if SWITCH
	if (strcmp("SWITCH",pcmd)==0) {
		pcmd = strtok(NULL," -,.");
		// SET SW1
		if (strcmp("SW1",pcmd)==0) {
			if (PORTD & (1<<PORTD7)) {
				PORTD &= ~(1<<PORTD7);
				strcpy(cmd,"OFF\0");
			} else {
				PORTD |= (1<<PORTD7);
				strcpy(cmd,"ON\0");
			}
			return;
		}
		// SWITCH SW2
		if (strcmp("SW2",pcmd)==0) {
			if (PORTD & (1<<PORTD6)) {
				PORTD &= ~(1<<PORTD6);
				strcpy(cmd,"OFF");
			} else  {
				PORTD |= (1<<PORTD6);
				strcpy(cmd,"ON");
			}
			return;
		}
	}
	// if SET
	if (strcmp("SET",pcmd)==0) {
		pcmd = strtok(NULL," -,.");
		// SET SW1
		if (strcmp("SW1",pcmd)==0) {
			PORTD |= (1<<PORTD7);
			strcpy(cmd,"ON\0");
			return;
		}
		// SET SW2
		if (strcmp("SW2",pcmd)==0) {
			PORTD |= (1<<PORTD6);
			strcpy(cmd,"ON");
			return;
		}
	}
	// if RESET
	if (strcmp("RESET",pcmd)==0) {
		pcmd = strtok(NULL," -,.");
		// RESET SW1
		if (strcmp("SW1",pcmd)==0) {
			PORTD &= ~(1<<PORTD7);
			strcpy(cmd,"OFF\0");
			return;
		}
		// RESET SW2
		if (strcmp("SW2",pcmd)==0) {
			PORTD &= ~(1<<PORTD6);
			strcpy(cmd,"OFF");
			return;
		}
	}
}

//========================================================
//
//========================================================
int main()
{
	int i=0;
#if ENABLE_UDP_RESPONSER
	uint8_t payloadlen=0;
#endif
    uint16_t plen;
    uint16_t dat_p;
    int8_t cmd;//, cmd_old=0;
    char * pchr;

	cli();
	//PORTB = 0xED; //
	DDRB  = 0xBF; //MISO line input, rest output

	//PORTD 7 output
	DDRD  = 0b11000000; //0xC0; //PORTD7 and PORTD6 output, rest input
	PORTD=0x00;
	//spi_init();
	MCUCR = 0x00;
	GICR  = 0x00;
	TIMSK = 0x00;
	delay_ms(50);

	uart_init();
	sei();                                     //enable global interrupts

	uart_send_string_from_FLASH(PSTR("Starting...\r\n")); 
	//uart_print("Starting...\n\r");
	wdt_enable(WDTO_1S);	// watchdog enabled

	timer_init();
	//pwm_init();

//INIT_SYSTEM:

	LCD_Initalize();
	LCD_Clear();
	LCD_WriteText("AVR Termometr");
	LCD_GoTo(0,1);
	LCD_WriteText("Init ...");

	//uart_printf("Init ENC28J60\n\r");
    /* set ETH_RESET to gnd -> reset the ethernet chip */
	cbi(ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_RESET); // low
	delay_ms(30);
    /* set output to Vcc, reset inactive */
    sbi(ENC28J60_CONTROL_PORT,ENC28J60_CONTROL_RESET); // high
    delay_ms(100);
    enc28j60Init(mymac);
    _delay_loop_1(50);
    //payloadlen=enc28j60getrev();
    //uart_printf("ENC28J60 Rev ID = B%d\n\r",payloadlen);
    /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
    // LEDB=yellow LEDA=green
    //
    // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
    // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
	enc28j60PhyWrite(PHLCON,0x476);
	_delay_loop_1(50);

	init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);
    // just use variable haveGWmac as a loop counter here to save memory
    while(haveGWmac<15){
    	if (enc28j60linkup()) {
    		haveGWmac=99;
    	}
    	i=0;
    	while(i<10){
    		_delay_loop_1(150); // 36ms
    		i++;
    	}
    	haveGWmac++;
    }
    haveGWmac=0;
	sei();

	// chcemy synchronizacji czasu
	NTPState = NEED_REQUEST;

    while(1){
    	wdt_reset();
    	plen = enc28j60PacketReceive(BUFFER_SIZE, buf);
    	// update NTP where is nothing to do.
        if(plen==0){
        	if (enc28j60linkup()){
        		if (haveGWmac==0){
                    client_arp_whohas(buf,gwip);
                    continue;
        		}
                if (NTPState == NEED_REQUEST){
                	init_ip_arp_udp_tcp(mymac,myip,2721);
					client_ntp_request(buf,ntpip,2721);
					//uart_printf("request NTP port:%d\r",2721);
					//init_ip_arp_udp_tcp(mymac,myip,123);
					NTPState = REQUEST_SEND;
					continue;
                }
        	}

        	//-----------------------------------
        	if (NTPState == RESPONSE_RECEIVED) {
                char day[16];
                char clock[12];
                if (winter_summer_time_correction(&time)==1)
            		{ uart_send_string_from_FLASH(PSTR("Zone : Summer time\r\n")); }
                else
            		{ uart_send_string_from_FLASH(PSTR("Zone : Winter time\r\n")); }
                gmtime(time+(int32_t)+360*(int32_t)hours_offset_to_utc,day,clock);
                //gmtime(time+(int32_t)+360*(int32_t)hours_offset_to_utc,,display_24hclock,day,clock);
                uart_printf("Day  : %s\r",day);
                uart_printf("Clock: %s\r",clock);
                //uart_printf("mamy odpowiedz z NTP\r");
        		NTPState = IDLE;
        	}
        	// linkup
        	continue;
        }

        // arp is broadcast if unknown but a host may also
        // verify the mac address by sending it to
        // a unicast address.
        if(eth_type_is_arp_and_my_ip(buf,plen)){
			if (eth_type_is_arp_req(buf)){
				make_arp_answer_from_request(buf);
			}
			if (eth_type_is_arp_reply(buf)){
				if (client_store_gw_mac(buf,gwip)){
					haveGWmac=1;
					//uart_printf("Have GW MAC address\r");
					//uart_printf("GW MAC = %02x:%02x:%02x:%02x:%02x:%02x\r",gwmacaddr[0],gwmacaddr[1],gwmacaddr[2],gwmacaddr[3],gwmacaddr[4],gwmacaddr[5]);
				}
			}
			continue;
        }

        // pakiet ICMP (ping)
		if(buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){
			// a ping packet, let's send pong
			make_echo_reply_from_request(buf,plen);
			continue;
        }

        // check if ip packets are for us:
        if(eth_type_is_ip_and_my_ip(buf,plen)==0){
        	continue;}

#if ENABLE_UDP_RESPONSER
		 // pakiet UDP
		 if (buf[IP_PROTO_P]==IP_PROTO_UDP_V && buf[UDP_DST_PORT_H_P]==(MYUDPPORT >>8) && buf[UDP_DST_PORT_L_P]==(MYUDPPORT & 0xFF)){
			 payloadlen=buf[UDP_LEN_L_P]-UDP_HEADER_LEN; // okresl dlugosc danych
			 //uart_printf("dlugosc: %d\n",payloadlen);
			 decode_udp_command(buf+UDP_DATA_P,command,payloadlen); // adres bufora + przesuniecie lokacji danych, rozmiar danych
			 continue;
		 }
#endif

#if ENABLE_WEB_SERVER
		 // pakiet TCP - serwer WWW
        // tcp port www start, compare only the lower byte
		if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==(MYWWWPORT>>8)&&buf[TCP_DST_PORT_L_P]==(MYWWWPORT &0xFF)){
			init_ip_arp_udp_tcp(mymac,myip,MYWWWPORT);
			if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V){
				make_tcp_synack_from_syn(buf);
				continue;
			}
			if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V){
				init_len_info(buf); // init some data structures
				// we can possibly have no data, just ack:
				dat_p=get_tcp_data_pointer();
				if (dat_p==0){
					if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V){
						make_tcp_ack_from_any(buf); // finack, answer with ack
					}
					// just an ack with no data, wait for next packet
					continue;
				}
				// jeli strona z has³em POST-owana (nie GET) czyli podano has³o...
				if (strncmp("POST /passwd",(char *)&(buf[dat_p]),12)==0){
					// head, post and other methods:
					//
					// for possible status codes see:
					// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
					pchr=strstr(&buf[dat_p],"Content-Length:"); // 15 len of that string!
					//uart_printf("Size:|%d|\r\n",atoi(&pchr[15])); //(0..14 is string)...15 is outside
					i=atoi(&pchr[15]);
					pchr=strstr(pchr,"\r\n\r\n"); // first is after Pragma:no-cache, we search after Content-Length...
					pchr=pchr+4;
//					{	uint8_t j=0;
//					 	while (j<i) {
//					 		uart_printf("%c",pchr[j]); // drukuj liniê komend (passwd=dupa+pawiana (urlencode/decode))
//					 	 	j++; }
//					}
					i=i-7;
					pchr=pchr+7; // omin 'passwd='
					if (strncmp(pchr,password,i)==0)
					{
						client_store_ip(buf,authorized_client_ip); // zapisz MAC klienta jako wiarygodny
						plen=moved_perm(buf);
					} else {
						plen=http401unauthorized();
					}
					goto SENDTCP;
				}

				// jeli strona z has³em POST-owana (nie GET) czyli podano has³o...
				if (strncmp("POST /ipconfig",(char *)&(buf[dat_p]),14)==0){
					// head, post and other methods:
					//
					// for possible status codes see:
					// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
					plen=print_ipconfig_page(buf );
					goto SENDTCP;

//					pchr=strstr(&buf[dat_p],"Content-Length:"); // 15 len of that string!
//					//uart_printf("Size:|%d|\r\n",atoi(&pchr[15])); //(0..14 is string)...15 is outside
//					i=atoi(&pchr[15]);
//					pchr=strstr(pchr,"\r\n\r\n"); // first is after Pragma:no-cache, we search after Content-Length...
//					pchr=pchr+4;
////					{	uint8_t j=0;
////					 	while (j<i) {
////					 		uart_printf("%c",pchr[j]); // drukuj liniê komend (passwd=dupa+pawiana (urlencode/decode))
////					 	 	j++; }
////					}
//					i=i-7;
//					pchr=pchr+7; // omin 'passwd='
//					if (strncmp(pchr,password,i)==0)
//					{
//						client_store_ip(buf,authorized_client_ip); // zapisz MAC klienta jako wiarygodny
//						plen=moved_perm(buf);
//					} else {
//						plen=http401unauthorized();
//					}
//					goto SENDTCP;
				}

				// jesli strony GET...
				if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0){
					// head, post and other methods:
					//
					// for possible status codes see:
					// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
					plen=http200ok();
					plen=fill_tcp_data_p(buf,plen,PSTR("<html><HEAD><BODY><H1>200 OK</h1></body></html>"));

					goto SENDTCP;
				}

				if ((strncmp("/style.css",(char *)&(buf[dat_p+4]),10)==0) || (strncmp("/sw/style.css",(char *)&(buf[dat_p+4]),13)==0)) {
					plen=print_style_css(buf);
					goto SENDTCP;
				}


				// strony/pliki bez autoryzacji musza byc wczesniej
				if (strncmp("/ ",(char *)&(buf[dat_p+4]),2)==0){
					plen=print_login_page(buf);
					goto SENDTCP;
				}

				if (is_client_authorized(buf,authorized_client_ip)==0) {
					uart_printf("Unauthorized entry event\r\n");
					plen=http200ok();
					plen=fill_tcp_data_p(buf,plen,PSTR("<html><HEAD><BODY><H1>Unauthorized!</h1></body></html>"));
					goto SENDTCP;
				}

//				for (int o=dat_p+4;o<dat_p+15;o++) {
//					uart_send(buf[o]);
//				}
//				uart_printf("\n\r");

				if ((strncmp("/menu.htm",(char *)&(buf[dat_p+4]),9)==0) || (strncmp("../menu.htm",(char *)&(buf[dat_p+4]),11)==0)) {
					plen=print_menu_page(buf );
					goto SENDTCP;
				}
				if ((strncmp("/ipconfig.htm",(char *)&(buf[dat_p+4]),13)==0) || (strncmp("../ipconfig.htm",(char *)&(buf[dat_p+4]),15)==0)) {
					plen=print_ipconfig_page(buf );
					goto SENDTCP;
				}

				//cmd_old=cmd;
				cmd=analyse_get_url((char *)&(buf[dat_p+5]));
				// for possible status codes see:
				// http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
				if (cmd==-1){
					plen=http401unauthorized();
					//plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Nieautoryzowany dostêp.</h1>"));
					goto SENDTCP;
				}

				if (cmd==-2){
					plen=print_webpage(buf,cmd);
					goto SENDTCP;
				}

				if (cmd==-3){
					// redirect to add a trailing slash
					plen=moved_perm(buf);
					goto SENDTCP;
				}

				if (cmd>=0) { // valid command?
					if (cmd==0) { PORTD &= ~(1<<PORTD6); PORTD &= ~(1<<PORTD7); }
					if (cmd==1) { PORTD &= ~(1<<PORTD6); PORTD |=  (1<<PORTD7); }
					if (cmd==2) { PORTD |=  (1<<PORTD6); PORTD &= ~(1<<PORTD7); }
				    if (cmd==3) { PORTD |=  (1<<PORTD6); PORTD |=  (1<<PORTD7); }
				    plen=print_webpage(buf,cmd);
					goto SENDTCP;
			 	}
SENDTCP:
				make_tcp_ack_from_any(buf); // send ack for http get
				make_tcp_ack_with_data(buf,plen); // send data
				continue;
			 }
		 }
#endif // #if ENABLE_WEB_SERVER

#if ENABLE_TCP_RESPONDER
		// pakiet TCP - komunikacja
        // tcp port www start, compare only the lower byte
		if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==(MYTCPPORT>>8)&&buf[TCP_DST_PORT_L_P]==(0xff & MYTCPPORT)){
			init_ip_arp_udp_tcp(mymac,myip,MYTCPPORT); // maly... workaround :)

			if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V){
				make_tcp_synack_from_syn(buf); //odebrano SYN, send SYN,ACK //	uart_printf("->SYN\r"); uart_printf("SYN, ACK->\r");
				continue;
			}

			if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V){  // jesli odebrano ACK, mamy dane //	uart_printf("->ACK\r");
				init_len_info(buf); // init some data structures
				// we can possibly have no data, just ack:
				dat_p=get_tcp_data_pointer();
				if (dat_p==0){
					if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V) { // odebrano FIN - zakoncz wysylajac ACK.
						make_tcp_ack_from_any(buf); // finack, answer with ack
					}
					continue; // just an ack with no data, wait for next packet
				}
				// okresl dlugosc lini danych (przeslanej komendy)... maks
			    i=(buf[IP_TOTLEN_H_P]<<8)|(buf[IP_TOTLEN_L_P]&0xff);
		        i-=IP_HEADER_LEN;
		        i-=(buf[TCP_HEADER_LEN_P]>>4)*4;
		        i-=2; // z jakiegos powodu -2 :(

				strncpy(command,&buf[dat_p],i);
				command[i]='\0';

				//uart_printf("command length=%d (B).\r",i);
				decode_execute_and_response_tcp_command((char *)command);
				plen=fill_tcp_data(buf,0,(char *)command); // to wysle, ale bez '\0'!!!
				// dlatego bardzo wa¿ne jest, zeby wyslany ciag mial znak konca wiersza,
				// inaczej klient (ReadLn) go nie odbierze !!!
				plen=fill_tcp_data_p(buf,plen,PSTR("\n")); // na koncu znak \n
//ANSWERTCP:
				make_tcp_ack_from_any(buf); // send ack for http get
				make_tcp_ack_with_data(buf,plen); // send data
				continue;
			}
		}

#endif // #if ENABLE_TCP_RESPONDER


		// ------------------------------------------------
		 // udp start, src port must be 123 (0x7b):
         if (buf[IP_PROTO_P]==IP_PROTO_UDP_V&&buf[UDP_SRC_PORT_H_P]==0&&buf[UDP_SRC_PORT_L_P]==0x7b){
        	 init_ip_arp_udp_tcp(mymac,myip,0x7b); // maly... workaround :)
             if (client_ntp_process_answer(buf,&time,NTPCLIENTPORT)){
                 // convert to unix time:
                 if ((time & 0x80000000UL) ==0){
                     // 7-Feb-2036 @ 06:28:16 UTC it will wrap:
                     time+=2085978496;
                 }else{
                     // from now until 2036:
                     time-=GETTIMEOFDAY_TO_NTP_OFFSET;
                 }
               	 NTPState = RESPONSE_RECEIVED;
             }
         }

    }
    return (0);
}
//---------

