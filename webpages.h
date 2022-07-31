#include <inttypes.h>
#include <avr/pgmspace.h>
#include "ip_arp_udp_tcp.h"

uint16_t print_style_css(char *buf); // style.css
uint16_t print_login_page(char *buf ); // login (default) page
uint16_t print_menu_page(unsigned char *buf ); // menu page
uint16_t print_ipconfig_page(char *buf);// IP configuation page
