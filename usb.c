#include "usb_desc.h"
#include "usb.h"
#include "usb_cdc.h"

static volatile BD_endpoint_t bdt[16]  __attribute__ ((aligned (512)));
static volatile ep_buffer_t bd_buf[4];

uint8_t state;
uint8_t ctl_stage;
uint8_t control_needs_zlp;
uint8_t dev_addr;
uint8_t status;

extern uint8_t cdc_state;

uint8_t current_cnf;
uint8_t alt_if;

static uint16_t wcount;
static char* ubuf;

static void setup_stage();
static void data_in_stage();
static void data_out_stage();
static void wait_for_setup_stage();

uint16_t usb_get_state() {
    return state;
}

void ep0_stall() {
    bdt[0].in.STAT.BSTALL = 1;
    bdt[0].in.STAT.UOWN=1;
    bdt[0].out.STAT.BSTALL = 1;
    bdt[0].out.STAT.UOWN=1;
    U1EP0bits.EPSTALL=1;
}

void usb_disengage_in_ep(uint16_t ep) {
    bdt[ep].in.STAT.UOWN = 0;
}

void usb_disengage_out_ep(uint16_t ep) {
    bdt[ep].out.STAT.UOWN = 0;
}

void usb_in_status(uint16_t ep) {
    bdt[ep].in.STAT.Val= 0x00;
    bdt[ep].in.STAT.Val |= _USIE | _DTSEN | _DAT1;
}

void usb_out_status(uint16_t ep) {
    bdt[ep].out.STAT.Val= 0x00;
    bdt[ep].out.STAT.Val |= _USIE | _DTSEN | _DAT1;
}

void usb_packet_tx(uint16_t ep, uint8_t* buf, uint16_t count) {
    memcpy(bdt[ep].in.ADR, buf, count);    
    bdt[ep].in.STAT.BC  = count & 0x3FF;
    bdt[ep].in.STAT.Val &= _DTSMASK;
    bdt[ep].in.STAT.DTS = ~bdt[ep].in.STAT.DTS;
    bdt[ep].in.STAT.Val |= _USIE | _DTSEN;
}

void usb_packet_rx(uint16_t ep, uint16_t count) {    
    bdt[ep].out.STAT.BC = count & 0x3FF;
    bdt[ep].out.STAT.Val &= _DTSMASK;
    bdt[ep].out.STAT.DTS = ~bdt[ep].out.STAT.DTS;
    bdt[ep].out.STAT.Val |= _USIE | _DTSEN;
}

void usb_configure_in_endpoint(uint16_t ep, uint16_t count, char* buf) {
    bdt[ep].in.STAT.Val = 0x00;
    bdt[ep].in.STAT.BC = count & 0x3FF;
    bdt[ep].in.ADR = buf;
}

void usb_configure_out_endpoint(uint16_t ep, uint16_t count, char* buf) {
    bdt[ep].out.STAT.Val = 0x00;
    bdt[ep].out.STAT.BC = count & 0x3FF;
    bdt[ep].out.ADR = buf;
}

void usb_reset() {
    memset(&bdt, 0x00, sizeof (bdt));
    memset(&bd_buf, 0x00, sizeof (bd_buf));

    U1IE = 0;
    U1EIE = 0;
    
    U1IR = 0xFF;
    U1EIR = 0xFF;
    
    U1IEbits.TRNIE = 1;
    U1IEbits.UERRIE = 1;
    U1IEbits.IDLEIE = 1;
    U1IEbits.RESUMEIE = 1;

    U1BDTP1 = (((unsigned int)bdt)>>8);
    U1CONbits.PPBRST = 1;
    U1CONbits.PPBRST = 0;
    U1CNFG1bits.PPB = 0b00;

    U1CON = 0x0000;
    U1CONbits.USBEN = 1;
    U1OTGCONbits.OTGEN = 0;

    usb_configure_in_endpoint(0, EP0_BUFF_SIZE, &bd_buf[0].in_buffer[0]);
    usb_configure_out_endpoint(0, EP0_BUFF_SIZE, &bd_buf[0].out_buffer[0]);

    U1EP0bits.EPTXEN = 1;
    U1EP0bits.EPRXEN = 1;
    U1EP0bits.EPHSHK = 1;
    U1EP0bits.EPCONDIS = 0;

    wait_for_setup_stage(); 
    
    U1PWRCbits.USBPWR = 1;
    state = ATTACHED;
    U1OTGCONbits.DPPULUP = 1;
}

void init_usb() {
    U1ADDR = 0;
    dev_addr = 0;
    usb_reset();
}

void ctl_send(char* data, uint16_t len) {
    wcount = len;
    ubuf = data;
    ctl_stage = DATA_IN;
    bdt[0].in.STAT.DTS=0;
    data_in_stage();
}

void ctl_recv(char* data, uint16_t len) {
    wcount = len;
    ubuf = data;
    ctl_stage = DATA_OUT;
    usb_packet_rx(0, EP0_BUFF_SIZE);
}

void ctl_ack() {
    ctl_stage = _STATUS;
    usb_in_status(0);
}

void cdc_init_endpoints(){
    usb_configure_in_endpoint(1, EP1_BUFF_SIZE, &bd_buf[1].in_buffer[0]);
    usb_configure_out_endpoint(1, EP1_BUFF_SIZE, &bd_buf[1].out_buffer[0]);
    usb_configure_in_endpoint(2, EP2_BUFF_SIZE, &bd_buf[2].in_buffer[0]);
    
    U1EP1bits.EPTXEN = 1;
    U1EP1bits.EPRXEN = 1;
    U1EP1bits.EPHSHK = 1;
    U1EP1bits.EPCONDIS = 0;
    U1EP2bits.EPTXEN = 1;
    U1EP2bits.EPRXEN = 0;
    U1EP2bits.EPHSHK = 1;
    U1EP2bits.EPCONDIS = 0;
    
    usb_packet_rx(1, EP1_BUFF_SIZE);
    usb_packet_tx(1, 0, 0);
    usb_packet_tx(2, 0, 0);
    
    cdc_state=CDC_READY;
}

static void process_standart_request(USB_SETUP_t* usb_setup) {
    uint16_t len = 0;
    uint16_t request = (usb_setup->bRequest | (usb_setup->bmRequestType << 8));
    switch (request) {
        case STD_CLEAR_FEATURE_INTERFACE:
        case STD_SET_FEATURE_INTERFACE:
        case STD_CLEAR_FEATURE_ENDPOINT:
        case STD_SET_FEATURE_ENDPOINT:
        case STD_CLEAR_FEATURE_ZERO:
        case STD_SET_FEATURE_ZERO:
            ctl_ack();
            break;
        case STD_GET_STATUS_INTERFACE:
            ctl_send(&status, 2);
            break;
        case STD_GET_INTERFACE:
            ctl_send(&alt_if, 1);
            break;
        case STD_SET_INTERFACE:
            ctl_ack();
            alt_if = usb_setup->wValueH;
            break;
        case STD_GET_STATUS_ENDPOINT:
            ctl_send(&status, 2);
            break;
        case STD_GET_STATUS_ZERO:
            ctl_send(&status, 2);
            break;
        case STD_SET_DESCRIPTOR:
            ctl_recv(0, usb_setup->wLen);
            break;
        case STD_GET_DESCRIPTOR_INTERFACE:
        case STD_GET_DESCRIPTOR_ENDPOINT:
        case STD_GET_DESCRIPTOR:
            len = usb_setup->wLen;
            switch (usb_setup->wValueH) {
                case USB_DEVICE_DESCRIPTOR_TYPE:
                    if (state == ADDRESSED) state = DEFAULT;
                    if (len>sizeof (device_dsc)) len = sizeof (device_dsc);
                    if (state == ATTACHED) len = 8;
                    ctl_send(&device_dsc, len);
                    break;
                case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                    if (len>sizeof (cfgDescriptor)) len = sizeof (cfgDescriptor);
                    ctl_send(&cfgDescriptor[0], len);
                    break;
                case USB_STRING_DESCRIPTOR_TYPE:
                    switch (usb_setup->wValueL) {
                        case 0:
                            if (len>sizeof (strLanguage)) len = sizeof (strLanguage);
                            ctl_send(&strLanguage[0], len);
                            break;
                        case 1:
                            if (len>sizeof (strManufacturer)) len = sizeof (strManufacturer);
                            ctl_send(&strManufacturer[0], len);
                            break;
                        case 2:
                            if (len>sizeof (strProduct)) len = sizeof (strProduct);
                            ctl_send(&strProduct[0], len);
                            break;
                        default:
                            ep0_stall();
                            //case 3:
                            //ctl_send(&strSerial[0], len));
                            //break;
                    }
                    break;
                case USB_INTERFACE_DESCRIPTOR_TYPE:
                    ctl_send(&cfgDescriptor[9], 0x9);
                    break;
                case USB_ENDPOINT_DESCRIPTOR_TYPE:
                    if (usb_setup->wValueL == 1) ctl_send(&cfgDescriptor[sizeof (cfgDescriptor) - 14], 0x7);
                    if (usb_setup->wValueL == 0x81) ctl_send(&cfgDescriptor[sizeof (cfgDescriptor) - 7], 0x7);
                    if (usb_setup->wValueL == 0x82) ctl_send(&cfgDescriptor[sizeof (cfgDescriptor) - 30], 0x7);
                    break;
                default:
                    ep0_stall();
            }
            break;
        case STD_GET_CONFIGURATION:
            ctl_send(&current_cnf, 1);
            break;
        case STD_SET_CONFIGURATION:
            state = CONFIGURED;
            cdc_init_endpoints();
            ctl_ack();
            break;
        case STD_SET_ADDRESS:
            state = ADDRESS_PENDING;
            dev_addr = usb_setup->wValueL;
            ctl_ack();
            break;
        default:
            ep0_stall();
    }
}

static void setup_stage(USB_SETUP_t* usb_setup) {
    ctl_stage = SETUP;
    usb_disengage_out_ep(0);
    usb_disengage_in_ep(0);

    switch (usb_setup->bmRequestType & 0x60) {
        case REQUEST_TYPE_STANDARD:
            process_standart_request(usb_setup);
            break;
        case REQUEST_TYPE_CLASS:
            process_cdc_request(usb_setup);
            break;
        default:
            ep0_stall();
    }
}

static void data_in_stage() {
    uint16_t tx_len = MIN(wcount, EP0_BUFF_SIZE);
    if (wcount > 0) {
        usb_packet_tx(0, ubuf, tx_len);    
        wcount -= tx_len;
        ubuf += tx_len;
    } else {
        if (control_needs_zlp) {
            control_needs_zlp = 0;
            usb_packet_tx(0, 0, 0);
        } else {
            ctl_stage = _STATUS;
            usb_out_status(0);
        }
    }    
}

static void data_out_stage() {
    uint16_t rx_len = bdt[0].out.STAT.BC;
    if (rx_len > 0) memcpy(ubuf, bdt[0].out.ADR, rx_len);
    ubuf += rx_len;
    wcount -= rx_len;
    if (wcount > 0) {
        usb_packet_rx(0, EP0_BUFF_SIZE);
    } else {
        ctl_stage = _STATUS;
        usb_in_status(0);
    }
}

static void wait_for_setup_stage() {
    ctl_stage = WAIT_SETUP;
    U1CONbits.PKTDIS = 0;
    control_needs_zlp = 0;

    bdt[0].out.STAT.DTS = 1;
    usb_packet_rx(0, EP0_BUFF_SIZE);
}

static void UnSuspend(void) {

}

static void Suspend(void) {
}

void poll_usb() {
    uint8_t ep;
    uint8_t pid;
    if (U1IRbits.URSTIF) {
        usb_reset();
        U1IRbits.URSTIF = 1;
    }
    if (U1IRbits.STALLIF) {
        bdt[0].in.STAT.BSTALL = 0;
        bdt[0].in.STAT.UOWN=0;
        bdt[0].out.STAT.BSTALL = 0;
        bdt[0].out.STAT.UOWN=0;
        U1EP0bits.EPSTALL=0;
        wait_for_setup_stage();
        U1IRbits.STALLIF=1;
    }
    while (U1IRbits.TRNIF) {
        ep = U1STATbits.ENDPT;
        if (U1STATbits.DIR) pid = bdt[ep].in.STAT.PID; else pid = bdt[ep].out.STAT.PID;
        switch (ep) { // endpoint
            case 0:
                switch (pid) {
                    case SETUP_PID: setup_stage(bdt[0].out.ADR);
                        break;
                    case OUT_PID:
                        if (ctl_stage == DATA_OUT) data_out_stage();
                        else if (ctl_stage == _STATUS) wait_for_setup_stage();
                        break;
                    case IN_PID:
                        if (state == ADDRESS_PENDING) {
                            state = ADDRESSED;
                            U1ADDR = dev_addr;
                        }
                        if (ctl_stage == DATA_IN) data_in_stage();
                        else if (ctl_stage == _STATUS) wait_for_setup_stage();
                        break;
                }
                break;
            case 1:
                if (pid == IN_PID) {
                    handle_cdc_in();
                }
                if (pid == OUT_PID) {
                    usb_packet_rx(1, EP0_BUFF_SIZE);
                }
                break;
            case 2:

                break;
            default:
                break;
        }
        U1CONbits.PKTDIS = 0;
        U1IRbits.TRNIF = 1;  
    }
    //U1IR=0x0001;
}