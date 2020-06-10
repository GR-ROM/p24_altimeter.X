/* 
 * File:   usb_cdc.h
 * Author: exp10der
 *
 * Created on July 14, 2018, 10:43 AM
 */

#ifndef USB_CDC_H
#define	USB_CDC_H

#include <stdint.h>
#include "usb.h"

/* Line Coding Structure */
#define LINE_CODING_LENGTH          0x07

typedef union _CONTROL_SIGNAL_BITMAP
{
    uint8_t _byte;
    struct
    {
        unsigned DTE_PRESENT;       // [0] Not Present  [1] Present
        unsigned CARRIER_CONTROL;   // [0] Deactivate   [1] Activate
    };
} CONTROL_SIGNAL_BITMAP;


/* Functional Descriptor Structure - See CDC Specification 1.1 for details */

/* Header Functional Descriptor */
typedef struct _USB_CDC_HEADER_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint16_t bcdCDC;
} USB_CDC_HEADER_FN_DSC;

/* Abstract Control Management Functional Descriptor */
typedef struct _USB_CDC_ACM_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint8_t bmCapabilities;
} USB_CDC_ACM_FN_DSC;

/* Union Functional Descriptor */
typedef struct _USB_CDC_UNION_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint8_t bMasterIntf;
    uint8_t bSaveIntf0;
} USB_CDC_UNION_FN_DSC;

/* Call Management Functional Descriptor */
typedef struct _USB_CDC_CALL_MGT_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
} USB_CDC_CALL_MGT_FN_DSC;

#define CDC_READY 0
#define CDC_BUSY 1

void process_cdc_request(USB_SETUP_t* usb_setup);
void send_cdc_buf(uint8_t* buf, uint8_t len);
void handle_cdc_out();
void handle_cdc_in();
void init_cdc();

#endif	/* USB_CDC_H */



