#include <xc.h>
#include "i2c.h"

#define TRIS_SDA TRISD
#define TRIS_SCL TRISD
#define PORT_SDA PORTD
#define PORT_SCL PORTD


#define SDA_PIN 9
#define SCL_PIN 10

#define PIN_SDA PORTD & (1<<SDA_PIN)
#define PIN_SCL PORTD & (1<<SCL_PIN)

#define SET_SDA{TRIS_SDA|=1<<SDA_PIN;}
#define CLR_SDA{TRIS_SDA&=~(1<<SDA_PIN);PORT_SDA&=~(1<<SDA_PIN);}
#define SET_SCL{TRIS_SCL|=1<<SCL_PIN;}
#define CLR_SCL{TRIS_SCL&=~(1<<SCL_PIN);PORT_SCL&=~(1<<SCL_PIN);}

/*typedef struct i2c{
    int sda;
    int scl;
    int id;
    int state;
};
 */
void i2c_init() {
    CLR_SDA
    CLR_SCL
}

/*
int i2c_open(int id, int mode)
{
    
}

void i2c_close(int handle)
{
    
}
 */

uint8_t i2c_read16(uint8_t addr, uint8_t reg, uint16_t* d){
    uint8_t t=0;
    *d=0;
    if (i2c_read(addr, reg, &t)==NACK) return NACK;
    *d=t<<8;
    t=0;
    if (i2c_read(addr, reg+1, &t)==NACK) return NACK;
    *d|=t;
    return ACK;
}


uint8_t static inline set_scl() {
    int timedout = 5000;
    SET_SCL;
    while (!PIN_SCL && timedout != 0) {
        timedout--;
    }
    if (timedout==0) return TIMEDOUT;
    return 0;
}

void static inline i2c_delay() {
    asm("nop");
}

void i2c_start() {
    set_scl();
    SET_SDA
    i2c_delay();
    CLR_SDA
    i2c_delay();
}

void i2c_stop() {
    CLR_SDA
    set_scl();
    i2c_delay();
    SET_SDA
    i2c_delay();
}

uint8_t i2c_send(uint8_t d){
    uint8_t i, r;
    for (i = 0; i != 8; i++) {
        CLR_SCL
        i2c_delay();
        if (d & 0x80) SET_SDA else CLR_SDA;
        d<<=1;
        i2c_delay();
        if (set_scl()==TIMEDOUT) return TIMEDOUT;
        i2c_delay();
    }
    CLR_SCL
    i2c_delay();
    SET_SDA
    i2c_delay();
    if (set_scl()==TIMEDOUT) return TIMEDOUT;
    i2c_delay();
    r = (PORTD & (1 << SDA_PIN));
    if (r == 0) r = ACK; else r = NACK;
    CLR_SCL
    return r;
}

uint8_t i2c_recv(uint8_t ack) {
    uint8_t i, d;
    d=0;
    for (i = 0; i != 8; i++) {
        CLR_SCL
        i2c_delay();
        if (PORT_SDA) d |= 1;
        d <<= 1;
        i2c_delay();
        if (set_scl()==TIMEDOUT) return TIMEDOUT;
        i2c_delay();
    }
    CLR_SCL
    i2c_delay();
    if (ack == ACK) CLR_SDA else SET_SDA;
    i2c_delay();
    if (set_scl()==TIMEDOUT) return TIMEDOUT;
    i2c_delay();
    CLR_SCL
    return d;
}

uint8_t i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
    i2c_start();
    if (i2c_send(dev_addr << 1) != ACK) return NACK;
    i2c_send(reg_addr);
    i2c_send(data);
    i2c_stop();
    return ACK;
}

uint8_t i2c_write_c(uint8_t dev_addr, uint8_t data) {
    i2c_start();
    if (i2c_send(dev_addr << 1) != ACK) return NACK;
    i2c_send(data);
    i2c_stop();
    return ACK;
}

uint8_t i2c_write_buf(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, uint8_t count) {
    uint8_t i; 
    i2c_start();
    if (i2c_send(dev_addr << 1) != ACK) return NACK;
    i2c_send(reg_addr);
    for (i = 0; i != count; i++) i2c_send(*(buf++));
    i2c_stop();
    return ACK;
}

uint8_t i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data) {
    i2c_start();
    if (i2c_send(dev_addr << 1) != ACK) return NACK;
    i2c_send(reg_addr);
    i2c_start();
    i2c_send(dev_addr << 1 | 1);
    *(data) = i2c_recv(NACK);
    i2c_stop();
    return ACK;
}

uint8_t i2c_read_buf(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, uint8_t count) {
    uint8_t i;
    if (count == 0) return NACK;
    i2c_start();
    if (i2c_send(dev_addr << 1) != ACK) return NACK;
    i2c_send(reg_addr);
    i2c_start();
    if (i2c_send(dev_addr << 1 | 1) != ACK) return NACK;
    if (count > 1) {
        count--;
        for (i = 0; i != count; i++) *(buf++) = i2c_recv(ACK);
    }
    *(buf++) = i2c_recv(NACK);
    i2c_stop();
    return ACK;
}