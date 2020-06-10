/* 
 * File:   i2c.h
 * Author: exp10
 *
 * Created on April 2, 2020, 6:05 PM
 */

#ifndef I2C_H
#define	I2C_H

#include <stdint.h>

#define ACK 0
#define NACK 1
#define TIMEDOUT 2
    
void i2c_init();
void i2c_start();
void i2c_stop();
uint8_t i2c_send(uint8_t d);
uint8_t i2c_recv(uint8_t ack);
uint8_t i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
uint8_t i2c_write_c(uint8_t dev_addr,  uint8_t data);
uint8_t i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data);
uint8_t i2c_read16(uint8_t addr, uint8_t reg, uint16_t* d);
uint8_t i2c_write_buf(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, uint8_t count);
uint8_t i2c_read_buf(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buf, uint8_t count);

#endif	/* I2C_H */