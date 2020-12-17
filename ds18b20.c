

#include "ds18b20.h"
#include "uart.h"

#include <string.h>

//###################################################################################
Ds18b20Sensor_t	ds18b20[_DS18B20_MAX_SENSORS];

uint8_t		Ds18b20TempSensorCount = 0;
uint8_t		Ds18b20StartConvert=0;
volatile uint16_t	Ds18b20Timeout=0;

//###########################################################################################
void Ds18b20_Init(OneWire_t *OneWire)
{
	Ds18b20TempSensorCount = 0;
	
	for(uint8_t i = 0; i < OneWire->DeviceCount; i++)
	{
		if(DS18B20_Is(&OneWire->Device[i].Address[0]))
		{
			memcpy(ds18b20[Ds18b20TempSensorCount].Address, OneWire->Device[i].Address, ROM_SIZE);
			Ds18b20TempSensorCount++;
		}
	}
}
//###########################################################################################
bool	Ds18b20_ManualConvert(void)
{
	char uartLog[100];

	Ds18b20Timeout = _DS18B20_CONVERT_TIMEOUT_MS / 10;
	DS18B20_Start(&OneWire, ds18b20[0].Address);
	ONEWIRE_DELAY(100);
	while (!DS18B20_AllDone(&OneWire))
	{
		ONEWIRE_DELAY(10);  
		Ds18b20Timeout -= 1;
	}	
	if(Ds18b20Timeout>0)
	{
		for (uint8_t i = 0; i < Ds18b20TempSensorCount; i++)
		{
			ONEWIRE_DELAY(100);
			ds18b20[i].DataIsValid = DS18B20_Read(&OneWire, &ds18b20[i]);
		}
	}
	else
	{
		for (uint8_t i = 0; i < Ds18b20TempSensorCount; i++)
		{
			ds18b20[i].DataIsValid = false;
		}
	}
	if(Ds18b20Timeout==0)
	{
		for (uint8_t i = 0; i < Ds18b20TempSensorCount; i++)
		{
			Uart_Println("invalid");
		}
		return false;
	}
	else
	{
		for (uint8_t i = 0; i < Ds18b20TempSensorCount; i++)
		{
			sprintf(uartLog, "%5.2f", ds18b20[i].Temperature);
			Uart_Println(uartLog);
		}
		return true;
	}
}
//###########################################################################################
uint8_t DS18B20_Start(OneWire_t* OneWire, uint8_t *ROM)
{
	/* Check if device is DS18B20 */
	if (!DS18B20_Is(ROM)) {
		return 0;
	}
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Start temperature conversion */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);
	
	return 1;
}

void DS18B20_StartAll(OneWire_t* OneWire)
{
	/* Reset pulse */
	OneWire_Reset(OneWire);
	/* Skip rom */
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);
	/* Start conversion on all connected devices */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);
}

bool DS18B20_Read(OneWire_t* OneWire, Ds18b20Sensor_t *sensor)
{
	uint16_t temperature;
	uint8_t resolution;
	int8_t digit, minus = 0;
	float decimal;
	uint8_t i = 0;
	uint8_t data[9];
	uint8_t crc;
	
	/* Check if device is DS18B20 */
	if (!DS18B20_Is(sensor->Address)) {
		return false;
	}
	
	/* Check if line is released, if it is, then conversion is complete */
	if (!OneWire_ReadBit(OneWire)) 
	{
		/* Conversion is not finished yet */
		return false; 
	}

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, sensor->Address);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, DS18B20_CMD_RSCRATCHPAD);
	
	/* Get data */
	for (i = 0; i < 9; i++) 
	{
		/* Read byte by byte */
		data[i] = OneWire_ReadByte(OneWire);
	}
	
	/* Calculate CRC */
	crc = OneWire_CRC8(data, 8);
	
	/* Check if CRC is ok */
	if (crc != data[8])
		/* CRC invalid */
		return 0;

	
	/* First two bytes of scratchpad are temperature values */
	temperature = data[0] | (data[1] << 8);

	/* Reset line */
	OneWire_Reset(OneWire);
	
	/* Check if temperature is negative */
	if (temperature & 0x8000)
	{
		/* Two's complement, temperature is negative */
		temperature = ~temperature + 1;
		minus = 1;
	}

	
	/* Get sensor resolution */
	resolution = ((data[4] & 0x60) >> 5) + 9;

	
	/* Store temperature integer digits and decimal digits */
	digit = temperature >> 4;
	digit |= ((temperature >> 8) & 0x7) << 4;
	
	/* Store decimal digits */
	switch (resolution) 
	{
		case 9:
			decimal = (temperature >> 3) & 0x01;
			decimal *= (float)DS18B20_DECIMAL_STEPS_9BIT;
		break;
		case 10:
			decimal = (temperature >> 2) & 0x03;
			decimal *= (float)DS18B20_DECIMAL_STEPS_10BIT;
		 break;
		case 11: 
			decimal = (temperature >> 1) & 0x07;
			decimal *= (float)DS18B20_DECIMAL_STEPS_11BIT;
		break;
		case 12: 
			decimal = temperature & 0x0F;
			decimal *= (float)DS18B20_DECIMAL_STEPS_12BIT;
		 break;
		default: 
			decimal = 0xFF;
			digit = 0;
	}
	
	/* Check for negative part */
	decimal = digit + decimal;
	if (minus) 
		decimal = 0 - decimal;
	
	
	/* Set to pointer */
	sensor->Temperature = decimal;
	
	/* Return 1, temperature valid */
	return true;
}

uint8_t DS18B20_GetResolution(OneWire_t* OneWire, Ds18b20Sensor_t *sensor)
{
	uint8_t conf;
	uint8_t buffer[9] = {0};
	
	if (!DS18B20_Is(sensor->Address))
		return 1;
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, sensor->Address);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, DS18B20_CMD_RSCRATCHPAD);
	
	for(uint8_t i = 0; i < 9; i++)
	{
		buffer[i] = OneWire_ReadByte(OneWire);
	}
	
	/* 5th byte of scratchpad is configuration register */
	conf = buffer[4];
	
	/* Return 9 - 12 value according to number of bits */
	sensor->resolution = (DS18B20_Resolution_t)(((conf & 0x60) >> 5) + 9);
	return 0;
}

uint8_t DS18B20_SetResolution(OneWire_t* OneWire, uint8_t *ROM, DS18B20_Resolution_t resolution) 
{
	uint8_t th, tl, conf;
	if (!DS18B20_Is(ROM)) 
		return 0;
	
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, DS18B20_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire);
	OneWire_ReadByte(OneWire);
	
	th = OneWire_ReadByte(OneWire);
	tl = OneWire_ReadByte(OneWire);
	conf = OneWire_ReadByte(OneWire);
	
	if (resolution == DS18B20_Resolution_9bits) 
	{
		conf &= ~(1 << DS18B20_RESOLUTION_R1);
		conf &= ~(1 << DS18B20_RESOLUTION_R0);
	}
	else if (resolution == DS18B20_Resolution_10bits) 
	{
		conf &= ~(1 << DS18B20_RESOLUTION_R1);
		conf |= 1 << DS18B20_RESOLUTION_R0;
	}
	else if (resolution == DS18B20_Resolution_11bits)
	{
		conf |= 1 << DS18B20_RESOLUTION_R1;
		conf &= ~(1 << DS18B20_RESOLUTION_R0);
	}
	else if (resolution == DS18B20_Resolution_12bits)
	{
		conf |= 1 << DS18B20_RESOLUTION_R1;
		conf |= 1 << DS18B20_RESOLUTION_R0;
	}
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	OneWire_WriteByte(OneWire, DS18B20_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, th);
	OneWire_WriteByte(OneWire, tl);
	OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t DS18B20_Is(uint8_t *ROM) 
{
	/* Checks if first byte is equal to DS18B20's family code */
	if (*ROM == DS18B20_FAMILY_CODE) 
		return 1;
	
	return 0;
}

uint8_t DS18B20_SetAlarmLowTemperature(OneWire_t* OneWire, uint8_t *ROM, int8_t temp) 
{
	uint8_t tl, th, conf;
	if (!DS18B20_Is(ROM)) 
		return 0;
	
	if (temp > 125) 
		temp = 125;
	 
	if (temp < -55) 
		temp = -55;
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, DS18B20_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire);
	OneWire_ReadByte(OneWire);
	
	th = OneWire_ReadByte(OneWire);
	tl = OneWire_ReadByte(OneWire);
	conf = OneWire_ReadByte(OneWire);
	
	tl = (uint8_t)temp; 

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	OneWire_WriteByte(OneWire, DS18B20_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, th);
	OneWire_WriteByte(OneWire, tl);
	OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t DS18B20_SetAlarmHighTemperature(OneWire_t* OneWire, uint8_t *ROM, int8_t temp) 
{
	uint8_t tl, th, conf;
	if (!DS18B20_Is(ROM)) 
		return 0;
	
	if (temp > 125) 
		temp = 125;
	
	if (temp < -55) 
		temp = -55;
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, DS18B20_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire);
	OneWire_ReadByte(OneWire);
	
	th = OneWire_ReadByte(OneWire);
	tl = OneWire_ReadByte(OneWire);
	conf = OneWire_ReadByte(OneWire);
	
	th = (uint8_t)temp; 

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	OneWire_WriteByte(OneWire, DS18B20_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, th);
	OneWire_WriteByte(OneWire, tl);
	OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t DS18B20_DisableAlarmTemperature(OneWire_t* OneWire, Ds18b20Sensor_t *sensor) 
{
	uint8_t tl, th, conf;
	if (!DS18B20_Is(sensor->Address)) 
		return 0;
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, sensor->Address);
	/* Read scratchpad command by onewire protocol */
	OneWire_WriteByte(OneWire, DS18B20_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	OneWire_ReadByte(OneWire);
	OneWire_ReadByte(OneWire);
	
	th = OneWire_ReadByte(OneWire);
	tl = OneWire_ReadByte(OneWire);
	conf = OneWire_ReadByte(OneWire);
	
	th = 125;
	tl = (uint8_t)-55;

	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, sensor->Address);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	OneWire_WriteByte(OneWire, DS18B20_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	OneWire_WriteByte(OneWire, th);
	OneWire_WriteByte(OneWire, tl);
	OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	OneWire_Reset(OneWire);
	/* Select ROM number */
	OneWire_SelectWithPointer(OneWire, sensor->Address);
	/* Copy scratchpad to EEPROM of DS18B20 */
	OneWire_WriteByte(OneWire, DS18B20_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t DS18B20_AlarmSearch(OneWire_t* OneWire)
{
	/* Start alarm search */
	return OneWire_Search(OneWire, DS18B20_CMD_ALARMSEARCH);
}

uint8_t DS18B20_AllDone(OneWire_t* OneWire)
{
	/* If read bit is low, then device is not finished yet with calculation temperature */
	return OneWire_ReadBit(OneWire);
}


