/*********************************************
 * This file can be used to decide which functionallity of the
 * TCP/IP stack shall be available. By picking the right functions
 * you can significantly reduce the size of the resulting code.
 *********************************************/

#ifndef IP_CONFIG_H
#define IP_CONFIG_H

//------------- functions in ip_arp_udp_tcp.c --------------
// an NTP client (ntp clock):
//#undef NTP_client
// a spontanious sending UDP client
//#define UDP_client
#define ENABLE_WEB_SERVER 1
#define ENABLE_UDP_RESPONSER 0
#define ENABLE_TCP_RESPONDER 0
// a server answering to UDP messages
//#define UDP_server

// to send out a ping:
//#undef PING_client
//#define PINGPATTERN 0x42

// a UDP wake on lan sender:
//#undef WOL_client

// function to send a gratuitous arp
//#define GRATARP

// a "web browser". This can be use to upload data
// to a web server on the internet by encoding the data
// into the url (like a Form action of type GET):
//#define WWW_client
// if you do not need a browser and just a server:
//#undef WWW_client
//
//------------- functions in websrv_help_functions.c --------------
//
// functions to decode cgi-form data:
//#define FROMDECODE_websrv_help

// function to encode a URL (mostly needed for a web client)
//#define URLENCODE_websrv_help

#endif /* IP_CONFIG_H */
