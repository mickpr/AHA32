#ifndef TIMECONVERSIONS_H
#define TIMECONVERSIONS_H

// Number of seconds between 1-Jan-1900 and 1-Jan-1970, unix time starts 1970
// and ntp time starts 1900.
#define GETTIMEOFDAY_TO_NTP_OFFSET 2208988800UL

extern uint8_t gmtime(const uint32_t time,char *day, char *clock);


#endif /* TIMECONVERSIONS_H */
