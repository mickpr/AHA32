#include "webpages.h"


uint16_t print_style_css(char *buf) {
	uint16_t plen;
	plen=fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/css\r\nPragma: no-cache\r\n\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("html, body { padding:0; margin:0;  height:100%; width:100%; }\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("html { display:table; }\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("body { display:table-cell; vertical-align:middle; text-align:center;\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("background: linear-gradient(to bottom right, #ffffff 0%, #404040 100%)}\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("#middle { width:400px; margin:0 auto; border:1px solid black; }\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("* html #middle { position:absolute; top:expression((x=(document.documentElement.offsetHeight-this.offsetHeight)/2)<0?0:x+'px'); left:50%;  margin-left:-160px;  }\r\n"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("div { border-radius:5px; width:350; font-family: Verdana; font-size: 16px; color: white; -moz-box-shadow: 3px 3px 5px 3px #000; -webkit-box-shadow: 3px 3px 5px 3px #000; box-shadow: 3px 3px 5px 3px #000; }"));
	plen=fill_tcp_data_p(buf,plen,PSTR("div {   width: 350; font-family: Verdana; font-size: 14px; font-weight: bold; }\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR(".divhead {   border-radius:5px 5px 0px 0px; background-color: #ffffff; color: #000000; -moz-box-shadow: 8px 8px 5px 3px #000; -webkit-box-shadow: 8px 8px 5px 3px #000; box-shadow: 8px 8px 5px 3px #000;}\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR(".divbody {   height:250; border-radius:0px 0px 5px 5px; background-color: #d0d0d0; color: #000000; -moz-box-shadow: 8px 8px 5px 3px #000; -webkit-box-shadow: 8px 8px 5px 3px #000; box-shadow: 8px 8px 5px 3px #000;}\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("table { font-family: Tahoma; font-size: 14px; font-weight: bold; }\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("A:link {text-decoration: none; color:white;}\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("A:visited {text-decoration: none; color:white;}\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("A:active {text-decoration: none; color:white;}\r\n"));
	plen=fill_tcp_data_p(buf,plen,PSTR("A:hover {text-decoration: none; color: black;}\r\n"));
	return (plen);
}

uint16_t print_login_page(char * buf ) {
	uint16_t plen;
	plen=http200ok();
	plen=fill_tcp_data_p(buf,plen,PSTR("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" /></head>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<form action=\"./passwd\" METHOD=\"POST\"><center>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divhead\">Home Automation - Please log-in:</div>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divbody\" style=\"height:60px;\"><br>Password: <input type=password value=\"\" name=\"passwd\"\\><input type=\"submit\" value=\"ENTER\"\\></div>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</center></form></body></html>"));
	return (plen);
}

uint16_t print_menu_page(unsigned char * buf ) {
	uint16_t plen;
	plen=http200ok();
	plen=fill_tcp_data_p(buf,plen,PSTR("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" /></head>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<body><center>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divhead\">Home Automation - Main Menu:</div>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divbody\"><br><A href=\"./sw/\">SWITCHES</a>&nbsp;<A href=\"./ipconfig.htm\">CONFIG</a></div>"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divbody\" style=\"height:60px;\"><br><A href=\"./sw/\">SWITCHES</a><br><A href=\"./ipconfig.htm\">CONFIG</a></div>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</center></body></html>"));
	return (plen);
}
uint16_t print_ipconfig_page(char * buf) {
	uint16_t plen;
	plen=http200ok();
	plen=fill_tcp_data_p(buf,plen,PSTR("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" /></head>"));
	//plen=fill_tcp_data_p(buf,plen,PSTR("<form action=\"./ipconfig\" METHOD=\"POST\"><center>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<body><center>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divhead\">Home Automation - IP Config:</div>"));

	plen=fill_tcp_data_p(buf,plen,PSTR("<div class=\"divbody\"><br>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<A href=\"../menu.htm\">MENU</a><BR>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<table><tr><td width=130>IP:</td><td>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='192' size=1 maxlen=3 name='ip1'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='168' size=1 maxlen=3 name='ip2'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='0' size=1 maxlen=3 name='ip3'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='100' size=1 maxlen=3 name='ip4'\\></td></tr>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Mask:</td><td>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='255' size=1 maxlen=3 name='mask1'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='255' size=1 maxlen=3 name='mask2'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='255' size=1 maxlen=3 name='mask3'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='0' size=1 maxlen=3 name='mask4'\\>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>Gateway:</td><td>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='192' size=1 maxlen=3 name='gw1'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='168' size=1 maxlen=3 name='gw2'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='0' size=1 maxlen=3 name='gw3'\\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='1' size=1 maxlen=3 name='gw4'\\>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td>NTP:</td><td>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='000' size=1 maxlen=3 name='nt1'\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='000' size=1 maxlen=3 name='nt2'\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='000' size=1 maxlen=3 name='nt3'\>."));
	plen=fill_tcp_data_p(buf,plen,PSTR("<input type=text value='000' size=1 maxlen=3 name='nt4'\>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("<tr><td></td><td><input type='submit' value='ENTER'\>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</td></tr></table><br></div>"));

	//plen=fill_tcp_data_p(buf,plen,PSTR("</center></form></html>"));
	plen=fill_tcp_data_p(buf,plen,PSTR("</center></body></html>"));
	return (plen);
}
