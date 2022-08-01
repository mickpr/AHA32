
-------- A U T O R Y Z A C J A --------
Mozna zrobic tak, ze podczas logowania (strona logowania) bedzie zapamietany MAC/IP klienta.  
Kazda strona bedzie odpowiadala jesli MAC/IP jest zgodny (strona logowania zawsze).
Po wylogowaniu zapamietany MAC/IP bedzie wykasowany!  To powinno zalatwic sprawe autoryzacji.
---------------------------------------
Plytka sterownika (AVR):
- Timer PWM + timer do DIMMER-ów - w sumie dowolny + przerwanie INTx
- Grafika (logo firmy ??)
- 2x DIMMER Triak
- 2x PWM Output (optotriac?)
- 2x odczyt stanu wejsc,
- switch konfiguracyjny + Reset,
- zewnêtrzna pamiêæ Flash??? RAM?? EEPROM?? SD?
- nazwa urzadzenia, zmiana nazwy, wyswietlanie na ekranie logowania (+IP???)
+ haslo, 
- zmiana hasla z poziomu przegl¹darki/TCP 
- zmiana adresu IP i portu do komunikacji TCP zprzegl¹darki i programu TCP
+ NTP (pobieranie czasu z serwera). 
- Diody sygnalizujace:
  - power
  - zsynchronizowanie czasu z serwerem
  - info (miganie, praca itd..)
  - stan 2 wyjsc
  - stan 2 wejsc (???)
 - wersja zasilana 12/5V ??? (przemyslec na PCB)
 
Program (PC): 
- definiowanie urzadzen wg IP/PORT
- kontrola polaczenia (ping?)
- autokonfiguracja (odczyt nazwy, parametrow systemu)
- sterowanie
 
Program (Android):
- definiowanie urzadzen wg IP/PORT
- autokonfiguracja (odczyt nazwy, parametrow systemu)
- sterowanie


//------------------------------------------------------------------------
// decode a url string e.g "hello%20joe" or "hello+joe" becomes "hello joe"
void urldecode(char *urlbuf)
{
        char c;
        char *dst;
        dst=urlbuf;
        while ((c = *urlbuf)) {
                if (c == '+') c = ' ';
                if (c == '%') {
                        urlbuf++;
                        c = *urlbuf;
                        urlbuf++;
                        c = (h2int(c) << 4) | h2int(*urlbuf);
                }
                *dst = c;
                dst++;
                urlbuf++;
        }
        *dst = '\0';
}
//------------------------------------------------------------------------
// There must be enough space in urlbuf. In the worst case that would be
// 3 times the length of str
void urlencode(const char *str,char *urlbuf)
{
        char c;
        while ((c = *str)) {
                if (c == ' '||isalnum(c)){ 
                        if (c == ' '){ 
                                c = '+';
                        }
                        *urlbuf=c;
                        str++;
                        urlbuf++;
                        continue;
                }
                *urlbuf='%';
                urlbuf++;
                int2h(c,urlbuf);
                urlbuf++;
                urlbuf++;
                str++;
        }
        *urlbuf='\0';
}
//------------------------------------------------------------------------

// mime encoded data for the led on and off images:
// see: http://www.motobit.com/util/base64-decoder-encoder.asp
//P(led_red) =  "<img src=\"data:image/png;base64,"
//		"iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAARnQU1BAACx"
//		"jwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAlwSFlz"
//		"AAALEgAACxIB0t1+/AAAAAd0SU1FB94FBhUWMCnH47MAAAIUSURBVDhPzZJbSNNhGMaFiupiZq26"
//		"GBV0ZB1grrJSd3JToVZLidDMC3WQFcxT2MY6UAldlBdRowIjZ8w/zRODTGW2xKZ0IctICGlhk8qL"
//		"oi6EIkP261t50TIx7/rgu/jgeX7P+77fm5DwX59T/fdW1gx716bcaJDPqdDe4YDxbaS77XOw4c0n"
//		"X+2HyKOrYX//rfu3fS26WUFPX3WWf3nu+477OjjtcGA/7NUy4ThE003HtzrJUzYjpHXAmzXeWwdF"
//		"R8HtgWAf2GyQqoFtKsKmrTgdJ2jp8O+bDjncNG8w5A5wMh+M2eBygSSB1QoaAdDqIW03V8xqqi+c"
//		"DwLz4yBl/saNI5JznJRk0BvAYoHcXDCZpgBaAdHxZM86ikqLv0YiY1viAOe6G1MHa0SaUgkGAUhP"
//		"F4lpv5kFQKNlSLWJI9YCenqChjjApa4uVf3xvIlJ5QYByBBpscQ/rk5HQL2ewpLiydejozvjAH0f"
//		"o7JqW+nLx6sVIl30HKviZ+9TEKMRdu3AuX0zVWfOhsUMkqYN0tXsraw06hmSyYRZtJCZ+WsGJmFW"
//		"JyOtUVCQn8cDv9/+168U1AXX6u94q4wGpMQkRhKXMCaX82LpMmqXr6AwJ4e7Hk9bNBpdOOMuCMgi"
//		"T+fDi6fLK97ZsrKx6TM4ZjZT4bC/b+1ovyzMi2fdxphACBW+UMjSHAyWtD8bOCjeq/7JOFfRD5WR"
//		"RAwNyTvlAAAAAElFTkSuQmCC"
//		"\"/>";
//		;

P(led_green) = "<img src=\"data:image/jpg;base64,"
		"iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABGdBTUEAALGPC/xhBQAAABl0RVh0"
		"U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAAIkSURBVDhPzZJfSFNRHMeFiuohCIIeopce"
		"eumll15Em2Nz947MOYqr2713/bn9eamcyXLTIFaZYI4KKm0xm7spbBXu1tDutju3u5uINSJQmqCs"
		"CfXiYw9REft2Zr0sE/OtA18OB76/z/me3/lVVf3XS+zr3xl/KO7xOHp2rCtoTo0bCnOTo+9mxz6o"
		"r0eWtOzIvCKJ4fDQsG5N0FtNcS4Ute+ZYgRDM33o0gQ40zbcS3UiMHDz6/BgsHVViPI8YsrNvIBv"
		"+hJeFp5hdikH35sO2JIGWBMHIQSa4HG1IRaNHVoBYRhmw3RaSt2acuNMqhHRuSBSBQndUxfAJyhw"
		"SRq2CSOsrSZ4O65oADZWQB713tk7Jvs/nxy3gFNMJHYzXCoHQTkMtgxI0OA1GhZfHc6eOP2lWCzu"
		"qwCEbj+oDjzuBhujwRKALV5PZIQ9YVouXgaoNKz9ehznHdDSmr4CEB4Q93u97d/YJ8Sc/nXjnyon"
		"aLxRh1O88GNxYfFABSCfz29zn29/b+00gNUo8IoZXPw3hOx8xgw2SYG263HZ1TVPerB9RSPFQfEi"
		"28KAvlYNLkvB8coMR5ZosvwsCtS5GvAMB1mW3X/9SkLd5L/vj/AtdtQLNWjorYXlrg4N13UwMrXg"
		"jtgQCoZGS6XS5lVngUC2RJ9Gr3qc7o/HmnnYj7bAwfDwtLk/SZLUQ4q3rjmNZQMx7sqkMhZ5XBZU"
		"VW0i593/VLhe009r7kDBwsJH0wAAAABJRU5ErkJggg=="
		"\"/>";
		;
P(led_gray) = "<img src=\"data:image/png;base64,"
		"iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAARnQU1BAACx"
		"jwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAlwSFlz"
		"AAALEgAACxIB0t1+/AAAAAd0SU1FB94DDxIdLCk1ypgAAAIWSURBVDhPzZJLaBNRFIYLKupCEAQX"
		"4saFGzdu3LgREdy40KCJyURjQwU3sbGTNB2KpWm78A1WxCS2c+eVmUlrTM0Yk2ZhbMf4jAhNfFRI"
		"pQnoRncuREXyezNYIdRSu/PC5XDhP9/5z7mnre2/PpFIePO4cnmbz+PbtCKjzx4b+6rvHk28Kj+Y"
		"f/7k7qeiOVbNGfyYqkp7lgWVnmbPzL9/8eP1m5cwzWncSiah6TLuGAJGYhe+KSLxLwnJG8r+cvk+"
		"7mUzmJmpoF6vI5fLgRABN0d4nL8ygFCoE5lM5sAiiMPhWFV8mCxks7chKypKpRIqlQoMw6AAAkmU"
		"IEgCvKfc6OvtLQJY3QKJXhzaPjkpfRkloxCpWNd1JBIJyLIMQRCsq6oqus924WRH+9darbajBRC7"
		"fml3XBpGNBalANGqyvO8FRcA8Xgc4aE+HPcwmJoq7m0BkCjZOTjQ83342lUoivInaSHZcqBpYHv8"
		"aPd4fs7N1Xe1AGZnP29g/aff+jt9iFOrkkR7/m29GZtQIhAcsdvAhUJVOoONiwYpEcK6XUfBsl1o"
		"2tVoxWbfmqZbMK/3BBinE/l8nvvrV1LqmkjkxribceEY4wbHcegP9yMYCOKwzQan3Q5ZlCcajcba"
		"JXeBQtalUqnB7kDwg9vFwOlw0KouCgl8TKfT52jy+mW3sSmgwi3ThcJBukgdpmkeou+t/5S4UtEv"
		"wJxbYorN/i4AAAAASUVORK5CYII="
		"\"/>";
		;





#include <avr/eeprom.h>

uint8_t eeprom_read_byte (const uint8_t *addr)
void eeprom_write_byte (uint8_t *addr, uint8_t value)
uint16_t eeprom_read_word (const uint16_t *addr)
void eeprom_write_word (uint16_t *addr, uint16_t value)
void eeprom_read_block (void *pointer_ram, const void *pointer_eeprom, size_t n)
void eeprom_write_block (void *pointer_eeprom, const void *pointer_ram, size_t n)

	uint8_t ByteOfData;
    ByteOfData = eeprom_read_byte((uint8_t*)46);

	uint16_t WordOfData;
    WordOfData = eeprom_read_word((uint16_t*)46);

	uint8_t StringOfData[10];
    eeprom_read_block((void*)&StringOfData, (const void*)12, 10);
------------------
#include <avr/eeprom.h>

uint8_t  EEMEM NonVolatileChar;
uint16_t EEMEM NonVolatileInt;
uint8_t  EEMEM NonVolatileString[10];

int main(void) {
    uint8_t SRAMchar;
    SRAMchar = eeprom_read_byte(&NonVolatileChar);
}
----------------------
#include <avr/eeprom.h>

uint8_t  EEMEM NonVolatileChar;
uint16_t EEMEM NonVolatileInt;
uint8_t  EEMEM NonVolatileString[10];

int main(void)
{
    uint8_t  SRAMchar;
    uint16_t SRAMint;

    SRAMchar = eeprom_read_byte(&NonVolatileChar);
    SRAMint  = eeprom_read_word(&NonVolatileInt);
}
-------------------------------------
#include <avr/eeprom.h>

uint8_t  EEMEM NonVolatileChar;
uint16_t EEMEM NonVolatileInt;
uint8_t  EEMEM NonVolatileString[10];

int main(void)
{
    uint8_t  SRAMchar;
    uint16_t SRAMint;
    uint8_t  SRAMstring[10];

    SRAMchar = eeprom_read_byte(&NonVolatileChar);
    SRAMint  = eeprom_read_word(&NonVolatileInt);
    eeprom_read_block((void*)&SRAMstring, (const void*)&NonVolatileString, 10);
}
-----------------
uint8_t EEMEM SomeVariable = 12;  // you must write eeprom!!! epp file while flashing
