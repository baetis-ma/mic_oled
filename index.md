## ESP32 with Microphone - Waveform and Soectrum Display
<img align="right" width="45%" height="350" src="hr.png"></img>
##### This project uses an ESP32S module attached to a PCF8591 ADC and an SSC1306 oled display, the ADC monitors the voltage output of an MAX9814 based microphone module. The oled display updates every 100msec with either the waveform of the sound detected by the microphone or it's sepctrum. The range of the display is about 12.5msec for the waveform and 0-2.5KHz for the spectrum.
##### This project uses Espressif IoT Developemnt Framework (esp-idf) to compile user application code, build the ESP code image and flash it into the ESP device. The setup is pretty easy and well documented on several websites (see user repository esp-idf-webpage for a walk through).

##### a good radix-2 fft can be found at https://github.com/fakufaku/esp32-fft

##### The ESP device establishes a wi-fi connection, then configures the max30102 to collect 100 samples per second of red and IR data. It then initiates two tasks:
##### adc_task() - a loop that
```
read ADC data 256 : 0.039911
calc 256 fft      : 0.002113
write oled        : 0.110118

```

