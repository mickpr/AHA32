################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dnslkup.c \
../enc28j60.c \
../ip_arp_udp_tcp.c \
../lcd.c \
../main.c \
../onewire.c \
../pwm.c \
../timeconv.c \
../timer.c \
../uart.c \
../webpages.c 

OBJS += \
./dnslkup.o \
./enc28j60.o \
./ip_arp_udp_tcp.o \
./lcd.o \
./main.o \
./onewire.o \
./pwm.o \
./timeconv.o \
./timer.o \
./uart.o \
./webpages.o 

C_DEPS += \
./dnslkup.d \
./enc28j60.d \
./ip_arp_udp_tcp.d \
./lcd.d \
./main.d \
./onewire.d \
./pwm.d \
./timeconv.d \
./timer.d \
./uart.d \
./webpages.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -Wno-deprecated-declarations -D__PROG_TYPES_COMPAT__ -mmcu=atmega32 -DF_CPU=11059200UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


