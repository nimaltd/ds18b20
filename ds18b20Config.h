#ifndef	_DS18B20CONFIG_H
#define	_DS18B20CONFIG_H


// ###################################################################################
// To FreeRTOS use 1 if CMSIS_V1 or 2 if CMSIS_V2
#define	_DS18B20_USE_FREERTOS		    			2

#define _DS18B20_MAX_SENSORS		    			1
#define	_DS18B20_GPIO								DS18B20_GPIO_Port
#define	_DS18B20_PIN								DS18B20_Pin

#define	_DS18B20_CONVERT_TIMEOUT_MS					5000
#if (_DS18B20_USE_FREERTOS==1||_DS18B20_USE_FREERTOS==2)
#define	_DS18B20_UPDATE_INTERVAL_MS					10000					//  (((	if==0  >> Ds18b20_ManualConvert()  )))    ((( if>0  >>>> Auto refresh )))
#endif


// ###################################################################################
// Init timer on cube    1us per tick				example 72 MHz cpu >>> Prescaler=(72-1)      counter period=Max
#define	_DS18B20_TIMER								htim10

#endif
