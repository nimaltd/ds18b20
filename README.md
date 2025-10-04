# 🌡️ DS18B20 Temperature Sensor Library for STM32  

A lightweight and **non-blocking DS18B20 driver** written in C for STM32 (HAL-based).  

Built on top of the **non-blocking 1-Wire library**, it allows reading temperature from one or multiple DS18B20 devices **without blocking the CPU**.  

It supports:  

- 🌡️ **DS18B20** – 9–12 bit temperature sensor (Celsius & Fahrenheit)  

The library is designed for:  

- Applications requiring **non-blocking temperature acquisition**  
- Multi-device support on a single 1-Wire bus  
- STM32 projects across F0/F1/F3/F4/F7/G0/G4/H7 families  

---

## ✨ Features  

- 🔹 Non-blocking operation using timer callbacks  
- 🔹 Supports multiple DS18B20 devices (`OW_MAX_DEVICE`)  
- 🔹 Configurable resolution (9–12 bit)  
- 🔹 Temperature read in Celsius and Fahrenheit  
- 🔹 Alarm threshold configuration  
- 🔹 Works with any GPIO pins  
- 🔹 Built on top of HAL-compatible **1-Wire library**  

---

## ⚙️ Installation  

### 1. Copy files directly  
Add these files to your STM32 project:  

- `ds18b20.h`  
- `ds18b20.c`  
- Ensure One-Wire Library added, https://github.com/nimaltd/ow  

### 2. STM32Cube Pack Installer (Recommended)  
Not yet available; include files manually for now.  

---

## 🔧 Configuration (`ds18b20_config_t`)  

Set conversion resolution and alarm thresholds (Not yet available):  

```c
ds18b20_config_t ds18_conf = {
    .alarm_high = 50,           // High temperature alarm
    .alarm_low  = -50,          // Low temperature alarm
    .cnv_bit    = DS18B20_CNV_BIT_12  // Resolution (9–12 bit)
};
```

---

## 🛠 CubeMX Setup  

Read in One-Wire Reopository

## 🚀 Quick Start  

### Include header  
```c
#include "ds18b20.h"
```

### Define a handle  
```c
ds18b20_handle_t ds18;
```

### Timer callback  
```c
void ds18_tim_cb(TIM_HandleTypeDef *htim)
{
    ow_callback(&ds18.ow);
}
```

### Optional done callback  
```c
void ds18_done_cb(ow_err_t error)
{
}
```

### Initialize DS18B20 in `main.c`  
```c
ow_init_t ow_init_struct;
ow_init_struct.tim_handle = &htim1;
ow_init_struct.gpio = GPIOC;
ow_init_struct.pin = GPIO_PIN_8;
ow_init_struct.tim_cb = ds18_tim_cb;
ow_init_struct.done_cb = NULL;   // Optional
ow_init_struct.rom_id_filter = DS18B20_ID;

ds18b20_init(&ds18, &ow_init_struct);

// Update ROM IDs for all devices
ds18b20_update_rom_id(&ds18);
while(ds18b20_is_busy(&ds18));

// Configure alarm thresholds and resolution
ds18b20_config_t ds18_conf = {
    .alarm_high = 50,
    .alarm_low = -50,
    .cnv_bit = DS18B20_CNV_BIT_12
};
ds18b20_conf(&ds18, &ds18_conf);
while(ds18b20_is_busy(&ds18));
```

### Read temperatures
```c
int16_t temp_c[2];

while(1) {
    ds18b20_cnv(&ds18);
    while(ds18b20_is_busy(&ds18));
    while(!ds18b20_is_cnv_done(&ds18));

    ds18b20_req_read(&ds18, 0);
    while(ds18b20_is_busy(&ds18));
    temp_c[0] = ds18b20_read_c(&ds18);

    ds18b20_req_read(&ds18, 1);
    while(ds18b20_is_busy(&ds18));
    temp_c[1] = ds18b20_read_c(&ds18);
}
```

---

## 🧰 API Overview  

| Function | Description |
|----------|-------------|
| `ds18b20_init()` | Initialize DS18B20 driver handle |
| `ds18b20_is_busy()` | Check if bus is busy |
| `ds18b20_last_error()` | Get last error |
| `ds18b20_update_rom_id()` | Update connected ROM IDs |
| `ds18b20_cnv()` | Start temperature conversion |
| `ds18b20_conf()` | Set configuration (alarm/resolution) |
| `ds18b20_is_cnv_done()` | Check if conversion is done |
| `ds18b20_req_read()` | Request temperature read |
| `ds18b20_read_c()` | Read temperature in Celsius |
| `ds18b20_read_f()` | Convert Celsius to Fahrenheit |

---

## 💖 Support  

If you find this project useful, please **⭐ star** the repo and support!  

- [![GitHub](https://img.shields.io/badge/GitHub-Follow-black?style=for-the-badge&logo=github)](https://github.com/NimaLTD)  
- [![YouTube](https://img.shields.io/badge/YouTube-Subscribe-red?style=for-the-badge&logo=youtube)](https://www.youtube.com/@nimaltd)  
- [![Instagram](https://img.shields.io/badge/Instagram-Follow-blue?style=for-the-badge&logo=instagram)](https://instagram.com/github.nimaltd)  
- [![LinkedIn](https://img.shields.io/badge/LinkedIn-Connect-blue?style=for-the-badge&logo=linkedin)](https://linkedin.com/in/nimaltd)  
- [![Email](https://img.shields.io/badge/Email-Contact-red?style=for-the-badge&logo=gmail)](mailto:nima.askari@gmail.com)  
- [![Ko-fi](https://img.shields.io/badge/Ko--fi-Support-orange?style=for-the-badge&logo=ko-fi)](https://ko-fi.com/nimaltd)  

---

## 📜 License  

Licensed under the terms in the [LICENSE](./LICENSE.TXT).  

---
