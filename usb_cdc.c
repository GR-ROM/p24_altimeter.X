#include "usb_cdc.h"
#include "usb.h"

#define FIFO_SIZE 64

uint8_t cdc_state;
LineCoding_t line;
char buffer[FIFO_SIZE];
char tx_buf[EP1_BUFF_SIZE];
uint8_t head;
uint8_t tail;
uint8_t count;

int put_fifo(uint8_t* d, uint8_t len){
    int t;
    if (len+count>FIFO_SIZE){
        return -1;
    }
    if (len+head>FIFO_SIZE) {
        t=FIFO_SIZE-head;
        memcpy(&buffer[head], &d[0], t);
        memcpy(&buffer[0], &d[t], (head+len)-FIFO_SIZE);
        head=(head+len)-FIFO_SIZE;
    } else {
        memcpy(&buffer[head], &d[0], len);
        head+=len;
    }    
    count+=len;
}

int get_fifo(uint8_t* d, uint8_t len){
    int t;
    if (count-len<0){
        return -1;
    }
    if (tail+len>FIFO_SIZE) {
        t=FIFO_SIZE-tail;
        memcpy(&d[0], &buffer[tail] , t);
        memcpy(&d[t], &buffer[0], (tail+len)-FIFO_SIZE);
        tail=(tail+len)-FIFO_SIZE;
    } else {
        memcpy(&d[0], &buffer[tail], len);    
        tail+=len;
    }    
    count-=len;
}

void init_cdc(){
    count=0;
    head=0;
    tail=0;
    cdc_state=CDC_BUSY;
}

void process_cdc_request(USB_SETUP_t* usb_setup){
    int request = (usb_setup->bRequest | (usb_setup->bmRequestType << 8));
    switch (request){
       case GET_LINE_CODING:
            ctl_send(&line, 7);
            break;  
        case SET_LINE_CODING:
            ctl_recv(&line, 7);
            break;  
        case SET_CONTROL_LINE_STATE:
            ctl_ack();
            break;   
        case SEND_BREAK:
            ctl_ack();
            break;
        case SEND_ENCAPSULATED_COMMAND:
            ctl_ack();
            break;
    }
}

void send_cdc_buf(uint8_t* buf, uint8_t len) {
    put_fifo(buf, len);
}

void handle_cdc_in(){    
    uint8_t pkt_len=MIN(count, EP1_BUFF_SIZE);
    if (count>0) {
        get_fifo(tx_buf, pkt_len);
        usb_packet_tx(1, tx_buf, pkt_len);
    } else usb_packet_tx(1, 0, 0);  
}

void handle_cdc_out(){
    
}