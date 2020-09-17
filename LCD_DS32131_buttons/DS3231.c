/*
 * DS3231.c
 *
 * Created: 26.02.2020 17:15:10
 *  Author: GaD_Bogdan
 */ 

#include "DS3231.h"

//===================================================================================================================
unsigned char RTC_ConvertFromDecToBD(unsigned char data)
{
	unsigned bdc_temp = (((data / 10) << 4) | (data % 10));
	return bdc_temp;
}
//===================================================================================================================
unsigned char RTC_ConvertFromBDToDec(unsigned char data)
{
	unsigned bdc_temp = ((data >> 4) * 10  +(0b00001111 & data));
	return bdc_temp;
}
//===================================================================================================================
