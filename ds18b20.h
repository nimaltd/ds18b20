
#ifndef	_DS18B20_H
#define	_DS18B20_H
//	2018/09/08


#include "onewire.h"
#include "ds18b20Config.h"
#include <stdbool.h>

#if (_DS18B20_USE_FREERTOS==1)
#include "cmsis_os.h"
#define	Ds18b20Delay(x)			osDelay(x)
#else
#define	Ds18b20Delay(x)			HAL_Delay(x)
#endif

//###################################################################################
typedef struct
{
	uint8_t 	Address[8];
	float 		Temperature;
	bool			DataIsValid;	
	
}Ds18b20Sensor_t;
//###################################################################################

extern Ds18b20Sensor_t	ds18b20[_DS18B20_MAX_SENSORS];

//###################################################################################
/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define DS18B20_FAMILY_CODE						0x28
#define DS18B20_CMD_ALARMSEARCH				0xEC

/* DS18B20 read temperature command */
#define DS18B20_CMD_CONVERTTEMP				0x44 	/* Convert temperature */
#define DS18B20_DECIMAL_STEPS_12BIT		0.0625
#define DS18B20_DECIMAL_STEPS_11BIT		0.125
#define DS18B20_DECIMAL_STEPS_10BIT		0.25
#define DS18B20_DECIMAL_STEPS_9BIT		0.5

/* Bits locations for resolution */
#define DS18B20_RESOLUTION_R1					6
#define DS18B20_RESOLUTION_R0					5

/* CRC enabled */
#ifdef DS18B20_USE_CRC	
#define DS18B20_DATA_LEN							9
#else
#define DS18B20_DATA_LEN							2
#endif

//###################################################################################
typedef enum {
	DS18B20_Resolution_9bits = 9,   /*!< DS18B20 9 bits resolution */
	DS18B20_Resolution_10bits = 10, /*!< DS18B20 10 bits resolution */
	DS18B20_Resolution_11bits = 11, /*!< DS18B20 11 bits resolution */
	DS18B20_Resolution_12bits = 12  /*!< DS18B20 12 bits resolution */
} DS18B20_Resolution_t;

//###################################################################################
#if (_DS18B20_USE_FREERTOS==1)
void			Ds18b20_Init(osPriority Priority);
#else
bool			Ds18b20_Init(void);
#endif
bool			Ds18b20_ManualConvert(void);
//###################################################################################
uint8_t 	DS18B20_Start(OneWire_t* OneWireStruct, uint8_t* ROM);
void 			DS18B20_StartAll(OneWire_t* OneWireStruct);
bool		 	DS18B20_Read(OneWire_t* OneWireStruct, uint8_t* ROM, float* destination);
uint8_t 	DS18B20_GetResolution(OneWire_t* OneWireStruct, uint8_t* ROM);
uint8_t 	DS18B20_SetResolution(OneWire_t* OneWireStruct, uint8_t* ROM, DS18B20_Resolution_t resolution);
uint8_t 	DS18B20_Is(uint8_t* ROM);
uint8_t 	DS18B20_SetAlarmHighTemperature(OneWire_t* OneWireStruct, uint8_t* ROM, int8_t temp);
uint8_t 	DS18B20_SetAlarmLowTemperature(OneWire_t* OneWireStruct, uint8_t* ROM, int8_t temp);
uint8_t 	DS18B20_DisableAlarmTemperature(OneWire_t* OneWireStruct, uint8_t* ROM);
uint8_t 	DS18B20_AlarmSearch(OneWire_t* OneWireStruct);
uint8_t 	DS18B20_AllDone(OneWire_t* OneWireStruct);
//###################################################################################

 
#endif

