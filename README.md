# ds18b20 library
<br />
I convert TM Library to Hal . www.stm32f4-discovery.net
<br />
I hope use it and enjoy.
<br />
I use Stm32f103vc and Keil Compiler and Stm32CubeMX wizard.
 <br />
Please Do This ...
<br />
<br />
1) Enable FreeRTOS  
<br />
2) Config a Gpio and a timer on CubeMX . 1us per tick				example 72 MHz cpu >>> Prescaler=(72-1)      counter period=0xFFFF  
<br />
3) Select "General peripheral Initalizion as a pair of '.c/.h' file per peripheral" on project settings.
<br />
4) Config your ds18b20Config.h file.
<br />
5) call  Ds18b20_Init(osPriorityNormal) on your app.
<br />
6) You can see result on debug . watch : ds18b20.

