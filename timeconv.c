// functions to convert ntp timestamps into time and date 

#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "uart.h"
// EPOCH = Jan 1 1970 00:00:00
#define	EPOCH_YR	1970
//(24L * 60L * 60L)
#define	SECS_DAY	86400UL
#define	LEAPYEAR(year)	(!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define	YEARSIZE(year)	(LEAPYEAR(year) ? 366 : 365)

extern volatile uint8_t sec,min,hour; // softwareowy zegar systemowy (zdefiniowany w timer.c)

static const char day_abbrev[] PROGMEM = "SunMonTueWedThuFriSat";

// isleapyear = 0-1
// month=0-11
// return: how many days a month has
//
// We could do this if ram was no issue:
//uint8_t monthlen(uint8_t isleapyear,uint8_t month){
//const uint8_t mlen[2][12] = {
//		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
//		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
//	};
//	return(mlen[isleapyear][month]);
//}
//
uint8_t monthlen(uint8_t isleapyear,uint8_t month){
	if(month==1){
		return(28+isleapyear);
	}
	if (month>6){
		month--;
	}
	if (month%2==1){
		return(30);
	}
	return(31);
}

// gmtime -- convert calendar time (sec since 1970) into broken down time
// returns something like Fri 2007-10-19 in day and 01:02:21 in clock
// The return values is the minutes as integer. This way you can update
// the entire display when the minutes have changed and otherwise just
// write current time (clock). That way an LCD display needs complete
// re-write only every minute.
uint8_t gmtime(const uint32_t time,char *day, char *clock)
{
    char dstr[4];
    uint8_t i;
	uint32_t dayclock;
    uint16_t dayno;
	uint16_t tm_year = EPOCH_YR;
	uint8_t tm_sec,tm_min,tm_hour,tm_wday,tm_mon;

	dayclock = time % SECS_DAY;
	dayno = time / SECS_DAY;

	tm_sec = dayclock % 60UL;
	tm_min = (dayclock % 3600UL) / 60;
	tm_hour = dayclock / 3600UL;

	// mickpr
	sec=tm_sec;
	hour=tm_hour;
	min=tm_min;
	// ------------mickpr
	tm_wday = (dayno + 4) % 7;	/* day 0 was a thursday */
	while (dayno >= YEARSIZE(tm_year)) {
		dayno -= YEARSIZE(tm_year);
		tm_year++;
	}
	tm_mon = 0;
	while (dayno >= monthlen(LEAPYEAR(tm_year),tm_mon)) {
		dayno -= monthlen(LEAPYEAR(tm_year),tm_mon);
		tm_mon++;
	}
    i=0;
    while (i<3){
            dstr[i]= pgm_read_byte(&(day_abbrev[tm_wday*3 + i]));
            i++;
    }
    dstr[3]='\0';

    sprintf_P(day,PSTR("%s %u-%02u-%02u"),dstr,tm_year,tm_mon+1,dayno + 1);  //TODO: tu nie uwzglednia zmian czasu zimowy/letni.
	sprintf_P(clock,PSTR("%02u:%02u:%02u"),tm_hour,tm_min,tm_sec);

    return(tm_min);
}

// ------------------ winter /summer time conversion
uint8_t winter_summer_time_correction(uint32_t * time) {
	uint32_t dayclock;
    uint16_t dayno;
	uint16_t tm_year = EPOCH_YR;
	//uint8_t tm_sec,tm_min;
	uint8_t tm_hour,tm_wday,tm_mon;

	dayclock = *time % SECS_DAY;
	dayno = *time / SECS_DAY;

	//tm_sec = dayclock % 60UL;
	//tm_min = (dayclock % 3600UL) / 60;
	tm_hour = dayclock / 3600UL;

	tm_wday = (dayno + 4) % 7;	/* day 0 was a thursday */
	while (dayno >= YEARSIZE(tm_year)) {
		dayno -= YEARSIZE(tm_year);
		tm_year++;
	}
	tm_mon = 0;
	while (dayno >= monthlen(LEAPYEAR(tm_year),tm_mon)) {
		dayno -= monthlen(LEAPYEAR(tm_year),tm_mon);
		tm_mon++;
	}

	if( (tm_mon > 3 && tm_mon < 10)	// miesiace od kwietnia do wrzesnia wlacznie
		|| (tm_mon == 3 && dayno > ( 31 - ( ( 31 - dayno + tm_wday ) % 7 ) ) )	// dni marca po ostaniej niedzieli
		|| (tm_mon == 3 && dayno == ( 31 - ( ( 31 - dayno + tm_wday ) % 7 ) ) && tm_hour > 0 )	// ostatnia niedziela marca, po godzinie 1.00
		|| (tm_mon == 10 && dayno < ( 31 - ( ( 31 - dayno + tm_wday ) % 7 ) ) )	// dni pazdziernika przed ostatnia niedziela
		|| (tm_mon == 10 && dayno == ( 31 - ( ( 31 - dayno + tm_wday ) % 7 ) ) && tm_hour < 1 ) )	// ostatnia niedziela pazdziernika, przed godzina 1.00
	{
		*time -= 3600;
		return 1;
	} else {
		return 0;
	}
}
