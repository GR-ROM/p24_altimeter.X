/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.166.1
        Device            :  PIC24FJ256GB106
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.41
        MPLAB 	          :  MPLAB X v5.30
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "mcc_generated_files/system.h"

/* Device header file */
#include <xc.h>
#include "usb.h"
#include "usb_cdc.h"
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "i2c.h"
#include "bmp085.h"
#include "pcf8531.h"
#define dprintf printf
/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/* i.e. uint16_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

int write(int handle, void *buffer, unsigned int len){
    send_cdc_buf(buffer, len);
    return (len);
}

int16_t main(void){
    bmp085_cal_t cal;
    /* Configure the oscillator for the device */
    dprintf("starting...\r\n");
    CLOCK_Initialize();
    dprintf("clock initialized\r\n");
    AD1PCFGL = 0x00;
    AD1PCFGH = 0x00;
    /* Initialize IO ports and peripherals */
    init_usb();
    init_cdc();
    dprintf("USB initialized\r\n");
    init_uc1601c();
    clear_lcd(0x0);
    point_lcd(1, 0, 0);

    /*  //  init_usb();
        #define debug
  #if defined debug
      cal.ac1=408;
      cal.ac2=-72;
      cal.ac3=-14383;
      cal.ac4=32741;
      cal.ac5=32757;
      cal.ac6=23153;
      cal.b1=6190;
      cal.b2=4;
      cal.mb=-32768;
      cal.mc=-8711;
      cal.md=2868;
      cal.ut=27898;
      cal.up=23843;
  #endif
      //   read_sensor_and_compute(&cal);
     */
    /* TODO <INSERT USER APPLICATION CODE HERE> */

    while(1) {
        poll_usb();
    }
}
