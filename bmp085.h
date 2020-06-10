/* 
 * File:   bmp085.h
 * Author: exp10
 *
 * Created on April 2, 2020, 6:16 PM
 */

#ifndef BMP085_H
#define	BMP085_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct bmp085_cal{
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int32_t b3;
    uint32_t b4;
    int32_t b5;
    int32_t b6;
    uint32_t b7;
    int16_t mb;
    int16_t mc;
    int16_t md;
    int32_t ut;
    int32_t up;
    int32_t x1;
    int32_t x2;
    int32_t x3;
    int32_t p;
    int16_t t;
    int16_t a;
} bmp085_cal_t;

int read_sensor_and_compute(bmp085_cal_t* cal);
int get_bmp085_altitude(float seaLevelPressure);


#ifdef	__cplusplus
}
#endif

#endif	/* BMP085_H */

