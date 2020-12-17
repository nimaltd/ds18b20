#ifndef _ONEWIRE_H_
#define _ONEWIRE_H_

#include "GeneralParameters.h"

#include <stdbool.h>

//	Init timer on cube    1us per tick				example 64 MHz cpu >>> Prescaler=(64-1)      counter period=Max 
//###################################################################################
#define ONEWIRE_TIMER			htim2
#define	OneWireDelay(x)			HAL_Delay(x)

#define ROM_SIZE    8
#define MAX_DEVICES 2

/* typedef of function pointer */
/***************************************************************************/
// setGPIO is a pointer to a function void foo(void)
typedef void (*setGPIO)(void);
// readGPIO is a pointer to a function uint16_t foo(void)
typedef uint8_t (*readGPIO)(void);

//###################################################################################
typedef struct
{
	uint8_t 	Address[ROM_SIZE];
	
}OneWireDevice_t;

//###################################################################################

typedef struct {
	setGPIO         SetLow;                /*!< Sets the OneWire GPIO Pin Low */
	setGPIO         SetFloating;           /*!< Lets the OneWire GPIO Pin floating */
	readGPIO        Read;                  /*!< Reads the OneWire GPIO Pin */
	uint8_t         LastDiscrepancy;       /*!< Search private */
	uint8_t         LastFamilyDiscrepancy; /*!< Search private */
	uint8_t         LastDeviceFlag;        /*!< Search private */
	uint8_t         ROM_NO[ROM_SIZE];      /*!< 8-bytes address of last search device */
	uint8_t         DeviceCount;           /*!< Number of connected devices */
	OneWireDevice_t Device[MAX_DEVICES];   /*!< Array of connected devices to the OneWire */
	uint8_t         crc8;
} OneWire_t;

//###################################################################################

extern OneWire_t OneWire;

/* OneWire delay */
bool ONEWIRE_DELAY(uint32_t time_us);

/* OneWire commands */
#define ONEWIRE_CMD_SEARCHROM      0xF0
#define ONEWIRE_CMD_READROM        0x33
#define ONEWIRE_CMD_MATCHROM       0x55
#define ONEWIRE_CMD_SKIPROM        0xCC
#define ONEWIRE_CMD_RESUME         0xA5

//#######################################################################################################
uint8_t OneWire_Init(OneWire_t* OneWire, setGPIO Low, setGPIO Float, readGPIO Read);
uint8_t OneWire_Reset(OneWire_t* OneWire);
uint8_t OneWire_SearchDevices(void);
uint8_t OneWire_ReadByte(OneWire_t* OneWire);
void    OneWire_WriteByte(OneWire_t* OneWire, uint8_t byte);
void    OneWire_WriteBit(OneWire_t* OneWire, uint8_t bit);
uint8_t OneWire_ReadBit(OneWire_t* OneWire);
uint8_t OneWire_Search(OneWire_t* OneWire, uint8_t command);
void    OneWire_ResetSearch(OneWire_t* OneWire);
uint8_t OneWire_First(OneWire_t* OneWire);
uint8_t OneWire_Next(OneWire_t* OneWire);
void    OneWire_GetFullROM(OneWire_t* OneWire, uint8_t *firstIndex);
void    OneWire_Select(OneWire_t* OneWire, uint8_t* addr);
void    OneWire_SelectWithPointer(OneWire_t* OneWire, uint8_t* ROM);
uint8_t OneWire_CRC8(uint8_t* addr, uint8_t len);
uint8_t docrc8(OneWire_t* OneWire, uint8_t value);
//#######################################################################################################
void    OneWire_SetGpioLow(void);
void    OneWire_SetGpioFloating(void);
uint8_t OneWire_ReadGpio(void);
//#######################################################################################################

#endif
