#include <string.h>
#include <stdio.h>

#include "math.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "../components/fft.h"
#include "../components/fft.c"
#define NUMSAMP 128    
void fft_calc(float *input, float *output, int num);
    
#define FB_LEN  1024  
uint8_t framebuffer[FB_LEN];

//i2c and periferal requirements
#include "driver/i2c.h"
static uint32_t i2c_frequency = 100000;
static i2c_port_t i2c_port = I2C_NUM_0;
static gpio_num_t i2c_gpio_sda = 18;
static gpio_num_t i2c_gpio_scl = 19;
#include "driver/gpio.h"
#define MODE_GPIO 5

#include "../components/i2c.h"
#include "../components/ssd1306.h"
#include "../components/gpio_setup.h"
#include "../components/disp_text.c"
#include "../components/disp_wave.c"

void fft_calc(float *input, float *output, int num)
{
    // Create fft plan and let it allocate arrays
    fft_config_t *fft_analysis = fft_init(num, FFT_COMPLEX, FFT_FORWARD, NULL, NULL);

    for (int k = 0 ; k < fft_analysis->size ; k++) {
      fft_analysis->input[2*k] = input[k];
      fft_analysis->input[2*k+1] = 0;
    }

    fft_execute(fft_analysis);
    for (int k = 0 ; k < fft_analysis->size/2 ; k++){
       output[k] = sqrt(fft_analysis->output[2*k]*fft_analysis->output[2*k]+
                        fft_analysis->output[2*k+1]*fft_analysis->output[2*k+1]);
    }
    //printf("\nraw fft output\n");
    //for (int k = 0 ; k < fft_analysis->size/2 ; k++) 
    //   printf("%4d %10.2f %10.2fj\n", k, fft_analysis->output[2*k], fft_analysis->output[2*k+1]); 
    fft_destroy(fft_analysis);
}

void app_main()
{
    float output[NUMSAMP];
    float input[NUMSAMP];
    uint8_t regdata[128];
    int mode;
    char *disp_str_wave="4  Waveform|||||||1  ~12ms full scale";
    char *disp_str_spec="4  Spectrum||||||||";

    i2cdetect();
    ssd1305_init();
    gpio_setup();

    while(1){
       //read mode pin on gpio 5
       mode=gpio_get_level(MODE_GPIO);

       //clear frame buffer, setup display text string and write text fields to frame buffer
       for(int i=0; i<FB_LEN; i++){ framebuffer[i]= (uint8_t)0x00; }
       if (mode == 1) display_text(disp_str_wave); else display_text(disp_str_spec);

       //read 128 bytes from ad convertor
       i2c_read(0x48, 0x00, regdata, NUMSAMP);
       //for (int a = 0; a < NUMSAMP; a++) printf("%d,", regdata[a]);
       //printf("\n");

       //write waveform to oled display framebuffer
       if (mode == 1) disp_wave(regdata);
       else {
           for (int a = 0; a < NUMSAMP; a++) input[a] = (float) regdata[a];
           fft_calc(input, output, NUMSAMP );

           //for (int k=0; k<NUMSAMP/2; k++)
           //   printf("%3d %10.2f\n", k, 0.01*output[k]);

          //write frambuffer with box for each frequency of spectrum
          uint32_t scratch;
          for(int a=2; a<64;a++){
             scratch =0;
             for(int s=0; s<64; s++) if(0.3*output[a/2]/8>s)scratch=scratch+(1<<(31-s));
             framebuffer[6*128 + a +30] = scratch/(1<<24);
             framebuffer[5*128 + a +30] = (scratch%(1<<24))/(1<<16);
             framebuffer[4*128 + a +30] = (scratch%(1<<16))/(1<<8);
             framebuffer[3*128 + a +30] = scratch%(1<<8);
           }
    }

       //write frame buffer to oled
       i2c_write_block( 0x3c, 0x40, framebuffer, FB_LEN);

       vTaskDelay(1);
   }
}

