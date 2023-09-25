#ifndef	_DS18B20CONFIG_H
#define	_DS18B20CONFIG_H


//	Init timer on cube    1us per tick				example 72 MHz cpu >>> Prescaler=(72-1)      counter period=Max 
//###################################################################################
#define	_DS18B20_USE_FREERTOS		    				1
#define _DS18B20_MAX_SENSORS		    				1
#define	_DS18B20_GPIO												DS18B20_GPIO_Port
#define	_DS18B20_PIN												DS18B20_Pin

#define	_DS18B20_CONVERT_TIMEOUT_MS					5000
#if (_DS18B20_USE_FREERTOS==1)
#define	_DS18B20_UPDATE_INTERVAL_MS					10000					//  (((	if==0  >> Ds18b20_ManualConvert()  )))    ((( if>0  >>>> Auto refresh )))
#endif

#define	_DS18B20_TIMER											htim13
#define _DS18B20_NO_NOT_USE_TIMER						1	/// use NOPs instead of hardware timer

#ifdef DEBUG
#define _DS18B20_CYCS_PER_NS							250	/// for(){NOP} cycles per nanosecond for 72 MHz core clock
#else
#define _DS18B20_CYCS_PER_NS							150	/// for(){NOP} cycles per nanosecond for 72 MHz core clock
#endif

//###################################################################################

#endif


