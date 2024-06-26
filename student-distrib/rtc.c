#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "scheduling.h"
#include "terminal.h"
uint32_t int_times;                 //num of interrupts in a second
uint32_t seconds;
uint32_t second;
uint32_t minute;
uint32_t hour;
uint32_t day;
uint32_t month;
uint32_t year;
uint32_t century;
int century_register = 0x00;                                // Set by ACPI table parsing code if possible
/* void RTC_init
 * initializa the RTC 
 * Inputs: uint8 rate
 * Return Value: void
 * side effect: enable IR8 in PIC*/
void RTC_init(){  
    char prev;
    second=0;
    minute=0;
    hour=0;
    day=0;
    month=0;
    year=0;
    terminal_t* local_term1=get_term(0);
    terminal_t* local_term2=get_term(1);
    terminal_t* local_term3=get_term(2);
    local_term1->freq=2;
    local_term1->interrupt=0;
    local_term2->freq=2;
    local_term2->interrupt=0;
    local_term3->freq=2;
    local_term3->interrupt=0;
    int_times=0;       
    seconds=0;
    outb(RTC_REG_B,RTC_PORT);		    // select register B, and disable NMI
    prev=inb(RTC_PORT_CMOS);	// initial value of register B
    outb(RTC_REG_B,RTC_PORT);		    // set the index again because a read will reset the index to register D
    outb(prev | 0x40, RTC_PORT_CMOS);	// turns on bit 6 of register B (UIE)   
    outb(RTC_REG_A,RTC_PORT);                   // select register A, disable NMI
    prev=inb(RTC_PORT_CMOS);	            // initial value of register A
    outb(RTC_REG_A,RTC_PORT);		            // set the index again because a read will reset the index to register D
    outb((prev & 0xF0) | 6,RTC_PORT_CMOS);   // write the rate to low 4 bits of Register A
                                            // we use 6 because the actual rate is 6, and actual frequency is 1024 
    enable_irq(RTC_IRQ_NUM);
}
/* rtc_open
 * open the pic
 * Inputs: a pointer to a file(file name)
 * Return Value: 0 on sucess
 * side effect: set the frequency to 2*/
int32_t rtc_open(const uint8_t* filename){
    get_term(cur_term)->freq=2;
    return 0;
}
/* rtc_read
 * read from the rtc
 * Inputs: a int32 fd, a pointer to a buffer, number of bytes we want
 * Return Value: 0 on sucess
 * side effect: none*/
int32_t rtc_read(int32_t fd,void* buf,int32_t nbytes){
    sti(); // ancient magic
    while(get_term(cur_term)->interrupt==0);   // wait until we get the interrupt
    get_term(cur_term)->interrupt=0;
    return 0;
}
/* rtc_write
 * write frequency to rtc
 * Inputs: a int32 fd, a pointer to a buffer, number of bytes we want 
 * Return Value: 0 on sucess,-1 on failure
 * side effect: none*/
int32_t rtc_write(int32_t fd, const void* buf,int32_t nbytes){
    uint32_t req_fren;
    terminal_t* local_term;
    local_term=get_term(cur_term);
   if(buf==NULL){
    return -1;
   }
    req_fren = *(uint32_t*) buf;
   if(((req_fren)&(req_fren-1))!=0){        //only when it is a power of two, the result will be 0
    return -1;
   }
   if(req_fren>1024){
    return -1;                          //biggest frequency we want 
   }
   if(nbytes!=4){                       //to many bytes
    return -1;
   }
  local_term->freq= req_fren;
   return 0;
}
/* void rtc_close
 * close the rtc
 * Inputs: int32 fd
 * Return Value: 0 on sucess
 * side effect: none*/
int32_t rtc_close(int32_t fd){
    return 0;
}
/* void handler_rtc
 * handle the interrupts generated by RTC
 * Inputs: void
 * Return Value: void
 * side effect: enable IR8 in PIC*/
void handler_rtc(){
    terminal_t* local_term1=get_term(0);
    terminal_t* local_term2=get_term(1);
    terminal_t* local_term3=get_term(2);
    int_times++;
    if(int_times%(1024/local_term1->freq)==0){   // 1024/frequency interrupts means we get one time interrupt in freqnecy we want
    local_term1->interrupt=1;}
    if(int_times%(1024/local_term2->freq)==0){   // 1024/frequency interrupts means we get one time interrupt in freqnecy we want
    local_term2->interrupt=1;}
    if(int_times%(1024/local_term3->freq)==0){   // 1024/frequency interrupts means we get one time interrupt in freqnecy we want
    local_term3->interrupt=1;}
    if(int_times%1024==0){
        seconds++;
    }
    outb((0xC), RTC_PORT);	// select register C
    inb(RTC_PORT_CMOS);		// just throw away contents
    send_eoi(RTC_IRQ_NUM);
}

/* get_frequency
 * get the frequency
 * Inputs: void
 * Return Value: frequency
 * side effect: none
*/
uint32_t get_frequency(){
    return 2;
}


/* get_global_time
 * Inputs: void
 * Return Value: none
 * side effect: none
*/                     
uint32_t get_update_in_progress_flag() {
      outb(0x0A,RTC_PORT);
      return (inb(RTC_PORT_CMOS) & 0x80);
}
 
uint32_t get_RTC_register(int reg) {
      outb(reg,RTC_PORT);
      return inb(RTC_PORT_CMOS);
}
// references from online source https://wiki.osdev.org/CMOS
void update_current_global_time(){
    uint8_t registerB;
    second = get_RTC_register(0x00);    //get the parameters from different ports
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);
    registerB = get_RTC_register(0x0B);
    second = (second & 0x0F) + ((second / 16) * 10);        //convert hex to dec
    minute = (minute & 0x0F) + ((minute / 16) * 10);
    hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
    day = (day & 0x0F) + ((day / 16) * 10);
    month = (month & 0x0F) + ((month / 16) * 10);
    year = (year & 0x0F) + ((year / 16) * 10);
    if (!(registerB & 0x02) && (hour & 0x80)) {         //from 12h to 24h
            hour = ((hour & 0x7F) + 12) % 24;
      }
    if(hour>6){                             //change the time zone to Chicago time zone
        hour=hour-6;
    }
    else{
        hour=hour+24-6;
        if(day>1){
            day--;}
        else{
            month--;
            day=30;
        }
    }
    year += (CURRENT_YEAR / 100) * 100;     //update the year the 20th century
    if(year < CURRENT_YEAR) year += 100;
}
