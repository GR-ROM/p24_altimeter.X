#include <stdint.h>
#include <math.h>
#include "bmp085.h"
#include "i2c.h"

#define __XTAL_FREQ 20000000U

char get_bmp085_calibration(bmp085_cal_t* cal){
    i2c_read16(0x77, 0xAA, (uint16_t*)&cal->ac1);
    i2c_read16(0x77, 0xAC, (uint16_t*)&cal->ac2);
    i2c_read16(0x77, 0xAE, (uint16_t*)&cal->ac3);
    i2c_read16(0x77, 0xB0, (uint16_t*)&cal->ac4);
    i2c_read16(0x77, 0xB2, (uint16_t*)&cal->ac5);
    i2c_read16(0x77, 0xB4, (uint16_t*)&cal->ac6);
    i2c_read16(0x77, 0xB6, (uint16_t*)&cal->b1);
    i2c_read16(0x77, 0xB8, (uint16_t*)&cal->b2);   
    i2c_read16(0x77, 0xBA, (uint16_t*)&cal->mb);
    i2c_read16(0x77, 0xBC, (uint16_t*)&cal->mc);
    i2c_read16(0x77, 0xBE, (uint16_t*)&cal->md);
    return 1;
}

char get_bmp085_ut(uint16_t* ut){
   *ut=0;
   if (i2c_write(0x77, 0xF4, 0x2E)==NACK) return 0;
//   __delay_ms(5);
   if (i2c_read16(0x77, 0xF6, ut)==NACK) return 0;
   return 1; 
}

char get_bmp085_up(int32_t* up){
   uint8_t t=0;
   if (i2c_write(0x34, 0xF4, 0x2E)==NACK) return 0;
//      __delay_ms(2);
   if (i2c_read(0x77, 0xF6, &t)==NACK) return 0;
   *up=t<<16;
   if (i2c_read(0x77, 0xF7, &t)==NACK) return 0;
   *up|=t<<8;
   if (i2c_read(0x77, 0xF8, &t)==NACK) return 0;
   *up|=t;
   *up>>=8;
   return 1; 
}

int compute_temp(bmp085_cal_t* cal){
    int32_t x1=((cal->ut-cal->ac6)*cal->ac5)>>15;
    int32_t x2=((int32_t)cal->mc<<11)/(x1+cal->md);
    cal->b5=x1+x2;
    int16_t t=(cal->b5+8)>>4;
    cal->ut=t;
    cal->t=t;
    return t;
}

void compute_pressure(bmp085_cal_t* cal){
    cal->b6 = cal->b5 - 4000;
    cal->x1 = ((int32_t)cal->b2 * ( (cal->b6 * cal->b6)>>12 )) >> 11;
    cal->x2 = ((int32_t)cal->ac2 * cal->b6) >> 11;
    cal->x3 = cal->x1 + cal->x2;
    cal->b3 = ((((int32_t)cal->ac1*4 + cal->x3)/* << oversampling*/) + 2) / 4;

    cal->x1 = ((int32_t)cal->ac3 * cal->b6) >> 13;
    cal->x2 = ((int32_t)cal->b1 * ((cal->b6 * cal->b6) >> 12)) >> 16;
    cal->x3 = ((cal->x1 + cal->x2) + 2) >> 2;
    cal->b4 = ((uint32_t)cal->ac4 * (uint32_t)(cal->x3 + 32768)) >> 15;
    cal->b7 = ((uint32_t)cal->up - cal->b3) * (uint32_t)( 50000UL /*>> oversampling }*/);

    if (cal->b7 < 0x80000000) cal->p = (cal->b7 * 2) / cal->b4;
    else cal->p = (cal->b7 / cal->b4) * 2;
    
    cal->x1 = (cal->p >> 8) * (cal->p >> 8);
    cal->x1 = (cal->x1 * 3038) >> 16;
    cal->x2 = (-7357 * cal->p) >> 16;

    cal->p = cal->p + ((cal->x1 + cal->x2 + (int32_t)3791)>>4);     
}

void compute_altitude(bmp085_cal_t* cal, float seaLevelPressure){
    cal->a=44330*(1-pow(cal->p/seaLevelPressure, 1/5.255));
}

int read_sensor_and_compute(bmp085_cal_t* cal){
    compute_temp(cal);
    compute_pressure(cal);
    compute_altitude(cal, 1013);
}