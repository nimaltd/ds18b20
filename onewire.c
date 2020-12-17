/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "onewire.h"
#include "uart.h"

OneWire_t OneWire;

bool ONEWIRE_DELAY(uint32_t time_us)
{
	ONEWIRE_TIMER.Instance->CNT = 0;
	while(ONEWIRE_TIMER.Instance->CNT <= time_us){
		__asm__ volatile ("");
	}
	return true;
}

// Initializes the 1-Wire bus
// returns 1 if initialization is still running
// returns 0 if initialization has finished
uint8_t OneWire_Init(OneWire_t* OneWire, setGPIO SetLow, setGPIO SetFloating, readGPIO Read) 
{	
	HAL_TIM_Base_Start_IT(&ONEWIRE_TIMER);
	
	// Hand over methods: SetLow, SetFloating and Read
	OneWire->SetLow = SetLow;
	OneWire->SetFloating = SetFloating;
	OneWire->Read = Read;
	
	OneWire->SetFloating();
	ONEWIRE_DELAY(1000000);
	OneWire->SetLow();
	// If the bus is held low for more than 480µs, all components on the bus will be reset.
	ONEWIRE_DELAY(1000000);
	OneWire->SetFloating();
	ONEWIRE_DELAY(2000000);
	return 1;
}

//###########################################################################################
uint8_t	OneWire_SearchDevices(void)
{
	uint8_t OneWireDevice = 0; 
	uint8_t	owDevicessTryToFind = 5;
	do
	{
		OneWire_Init(&OneWire, OneWire_SetGpioLow, OneWire_SetGpioFloating, OneWire_ReadGpio);
		OneWire.DeviceCount = 0;	
		while(HAL_GetTick() < 3000)
			ONEWIRE_DELAY(100);
		OneWireDevice = OneWire_First(&OneWire);
		while (OneWireDevice)
		{
			ONEWIRE_DELAY(100);
			OneWire.DeviceCount++;
			OneWire_GetFullROM(&OneWire, OneWire.Device[OneWire.DeviceCount-1].Address);
			OneWireDevice = OneWire_Next(&OneWire);
		}
		if(OneWire.DeviceCount>0)
			break;
		owDevicessTryToFind--;
	}while(owDevicessTryToFind>0);
	if(owDevicessTryToFind==0)
		return 0;
	return OneWire.DeviceCount;
}
//###########################################################################################

// Resets the 1-Wire bus
// returns 2 if detection still running
// returns 1 if no presence detected
// returns 0 if presence detected
uint8_t OneWire_Reset(OneWire_t* OneWire)
{
	uint8_t i;
	
	/* Line low, and wait 480us */
	OneWire->SetLow();
	ONEWIRE_DELAY(480);
	ONEWIRE_DELAY(20);
	/* Release line and wait for 70us */
	OneWire->SetFloating();
	ONEWIRE_DELAY(70);
	/* Check bit value */
	i = OneWire->Read();
	
	/* Delay for 410 us */
	ONEWIRE_DELAY(410);
	/* Return value of presence pulse, 0 = OK, 1 = ERROR */
	return i;
}

inline void OneWire_WriteBit(OneWire_t* OneWire, uint8_t bit)
{
	uint32_t delay_low;
	uint32_t delay_high;
	
	if(bit) {
		delay_low = 10;
		delay_high = 55;
	}
	else {
		delay_low = 65;
		delay_high = 5;
	}
	
	/* Set line low */
	OneWire->SetLow();
	/* Wait for 10us for bit == 1 or 65us for bit == 0 and release the line */
	ONEWIRE_DELAY(delay_low);
	
	/* Bit high */
	 OneWire->SetFloating();
	
	/* Wait for 55us for bit == 1 or 5us for bit == 0 and release the line */
	ONEWIRE_DELAY(delay_high);
}

inline uint8_t OneWire_ReadBit(OneWire_t* OneWire) 
{
	uint8_t bit = 0;
	
	/* Line low */
	OneWire->SetLow();
	ONEWIRE_DELAY(2);
	
	/* Release line */
	OneWire->SetFloating();
	ONEWIRE_DELAY(10);
	
	/* Read line value */
	if (OneWire->Read()) {
		/* Bit is HIGH */
		bit = 1;
	}
	
	/* Wait 50us to complete 60us period */
	ONEWIRE_DELAY(50);
	
	/* Return bit value */
	return bit;
}

void OneWire_WriteByte(OneWire_t* OneWire, uint8_t byte) {
	uint8_t i;
	
	/* Write 8 bits */
	for(i = 0; i < 8; i++) {
		/* LSB bit is first */
		OneWire_WriteBit(OneWire, byte & 0x01);
		byte = byte >> 1;
	}
}

uint8_t OneWire_ReadByte(OneWire_t* OneWire) {
	uint8_t i;
	uint8_t byte = 0;
	
	/* Read 8 bits */
	for(i = 0; i < 8; i++) {
		byte = byte >> 1;
		byte |= (OneWire_ReadBit(OneWire) << 7);
	}
	
	return byte;
}

uint8_t OneWire_First(OneWire_t* OneWire) {
	/* Reset search values */
	OneWire_ResetSearch(OneWire);

	/* Start with searching */
	return OneWire_Search(OneWire, ONEWIRE_CMD_SEARCHROM);
}

uint8_t OneWire_Next(OneWire_t* OneWire) {
   /* Leave the search state alone */
   return OneWire_Search(OneWire, ONEWIRE_CMD_SEARCHROM);
}

void OneWire_ResetSearch(OneWire_t* OneWire) {
	/* Reset the search state */
	OneWire->LastDiscrepancy = 0;
	OneWire->LastDeviceFlag = 0;
	OneWire->LastFamilyDiscrepancy = 0;
}

uint8_t OneWire_Search(OneWire_t* OneWire, uint8_t command) {
	uint8_t id_bit_number;
	uint8_t last_zero;
	uint8_t rom_byte_number;
	uint8_t search_result;
	uint8_t id_bit;
	uint8_t cmp_id_bit;
	uint8_t rom_byte_mask;
	uint8_t search_direction;

	/* Initialize for search */
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;
	OneWire->crc8 = 0;

	// if the last call was not the last one
	if (!OneWire->LastDeviceFlag)
	{
		// 1-Wire reset
		if (OneWire_Reset(OneWire)) 
		{
			/* Reset the search */
			OneWire->LastDiscrepancy = 0;
			OneWire->LastDeviceFlag = 0;
			OneWire->LastFamilyDiscrepancy = 0;
			return 0;
		}

		// issue the search command 
		OneWire_WriteByte(OneWire, command);  

		// loop to do the search
		do {
			// read a bit and its complement
			id_bit = OneWire_ReadBit(OneWire);
			cmp_id_bit = OneWire_ReadBit(OneWire);

			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1)) {
				break;
			} else {
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit) {
					search_direction = id_bit;  // bit write value for search
				} else {
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < OneWire->LastDiscrepancy) {
						search_direction = ((OneWire->ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					} else {
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == OneWire->LastDiscrepancy);
					}
					
					// if 0 was picked then record its position in LastZero
					if (search_direction == 0) {
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9) {
							OneWire->LastFamilyDiscrepancy = last_zero;
						}
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1) {
					OneWire->ROM_NO[rom_byte_number] |= rom_byte_mask;
				} else {
					OneWire->ROM_NO[rom_byte_number] &= ~rom_byte_mask;
				}
				
				// serial number search direction write bit
				OneWire_WriteBit(OneWire, search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0) {
					docrc8(OneWire, OneWire->ROM_NO[rom_byte_number]);  // accumulate the CRC
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		} while (rom_byte_number < 8);  // loop until through all ROM bytes 0-7

		// if the search was successful then
		if (!((id_bit_number < 65) || (OneWire->crc8 != 0))) {
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			OneWire->LastDiscrepancy = last_zero;

			// check for last device
			if (OneWire->LastDiscrepancy == 0) {
				OneWire->LastDeviceFlag = 1;
			}

			search_result = 1;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !OneWire->ROM_NO[0]) {
		OneWire->LastDiscrepancy = 0;
		OneWire->LastDeviceFlag = 0;
		OneWire->LastFamilyDiscrepancy = 0;
		search_result = 0;
	}

	return search_result;
}

int OneWire_Verify(OneWire_t* OneWire) {
	unsigned char rom_backup[8];
	int i;
	int rslt;
	int ld_backup;
	int ldf_backup;
	int lfd_backup;

	// keep a backup copy of the current state
	for (i = 0; i < 8; i++) {
		rom_backup[i] = OneWire->ROM_NO[i];
	}
	ld_backup = OneWire->LastDiscrepancy;
	ldf_backup = OneWire->LastDeviceFlag;
	lfd_backup = OneWire->LastFamilyDiscrepancy;

	// set search to find the same device
	OneWire->LastDiscrepancy = 64;
	OneWire->LastDeviceFlag = 0;

	if (OneWire_Search(OneWire, ONEWIRE_CMD_SEARCHROM)) {
		// check if same device found
		rslt = 1;
		for (i = 0; i < 8; i++) {
			if (rom_backup[i] != OneWire->ROM_NO[i]) {
				rslt = 1;
				break;
			}
		}
	} else {
		rslt = 0;
	}

	// restore the search state 
	for (i = 0; i < 8; i++) {
		OneWire->ROM_NO[i] = rom_backup[i];
	}
	OneWire->LastDiscrepancy = ld_backup;
	OneWire->LastDeviceFlag = ldf_backup;
	OneWire->LastFamilyDiscrepancy = lfd_backup;

	// return the result of the verify
	return rslt;
}

void OneWire_TargetSetup(OneWire_t* OneWire, uint8_t family_code) {
   uint8_t i;

	// set the search state to find SearchFamily type devices
	OneWire->ROM_NO[0] = family_code;
	for (i = 1; i < 8; i++) {
		OneWire->ROM_NO[i] = 0;
	}
	
	OneWire->LastDiscrepancy = 64;
	OneWire->LastFamilyDiscrepancy = 0;
	OneWire->LastDeviceFlag = 0;
}

void OneWire_FamilySkipSetup(OneWire_t* OneWire) {
	// set the Last discrepancy to last family discrepancy
	OneWire->LastDiscrepancy = OneWire->LastFamilyDiscrepancy;
	OneWire->LastFamilyDiscrepancy = 0;

	// check for end of list
	if (OneWire->LastDiscrepancy == 0) {
		OneWire->LastDeviceFlag = 1;
	}
}

uint8_t OneWire_GetROM(OneWire_t* OneWire, uint8_t index) {
	return OneWire->ROM_NO[index];
}

void OneWire_Select(OneWire_t* OneWire, uint8_t* addr) {
	uint8_t i;
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_MATCHROM);
	
	for (i = 0; i < 8; i++) {
		OneWire_WriteByte(OneWire, *(addr + i));
	}
}

void OneWire_SelectWithPointer(OneWire_t* OneWire, uint8_t *ROM) {
	uint8_t i;
	OneWire_WriteByte(OneWire, ONEWIRE_CMD_MATCHROM);
	
	for (i = 0; i < 8; i++) {
		OneWire_WriteByte(OneWire, *(ROM + i));
	}	
}

void OneWire_GetFullROM(OneWire_t* OneWire, uint8_t *firstIndex) {
	uint8_t i;
	for (i = 0; i < 8; i++) {
		*(firstIndex + i) = OneWire->ROM_NO[i];
	}
}

uint8_t OneWire_CRC8(uint8_t *addr, uint8_t len) {
	uint8_t crc = 0;
	uint8_t inbyte;
	uint8_t i;
	uint8_t j;
	uint8_t mix;
	
	for(j = 0; j < len; j++) {
		inbyte = *addr++;
		for (i = 8; i; i--) {
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) {
				crc ^= 0x8C;
			}
			inbyte >>= 1;
		}
	}
	
	/* Return calculated CRC */
	return crc;
}

// TEST BUILD
static uint8_t dscrc_table[] = {
    0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
	157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
   35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
  190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
   70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
  219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
  101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
  248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
  140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
   17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
  175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
   50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
  202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
   87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
  233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
  116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current 
// global 'crc8' value. 
// Returns current global crc8 value
//
uint8_t docrc8(OneWire_t* OneWire, uint8_t value)
{
   // See Application Note 27
   
   // TEST BUILD
   OneWire->crc8 = dscrc_table[OneWire->crc8 ^ value];
   return OneWire->crc8;
}

/* hardware access funcions for 1-wire */
/***************************************************************************/
void OneWire_SetGpioLow(void){
	HAL_GPIO_WritePin(ONE_WIRE_GPIO_Port, ONE_WIRE_Pin, GPIO_PIN_RESET);
}

void OneWire_SetGpioFloating(void){
  HAL_GPIO_WritePin(ONE_WIRE_GPIO_Port, ONE_WIRE_Pin, GPIO_PIN_SET);
}

uint8_t OneWire_ReadGpio(void){
	return (uint8_t)HAL_GPIO_ReadPin(ONE_WIRE_GPIO_Port, ONE_WIRE_Pin);
}

