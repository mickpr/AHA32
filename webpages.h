#include <inttypes.h>
#include <avr/pgmspace.h>
#include "ip_arp_udp_tcp.h"

uint16_t http200ok(uint8_t *buf);
uint16_t print_style_css(uint8_t *buf); // style.css
uint16_t print_login_page(uint8_t *buf ); // login (default) page
uint16_t print_menu_page(unsigned char *buf ); // menu page
uint16_t print_ipconfig_page(uint8_t *buf);// IP configuation page
