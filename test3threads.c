/* 
 * file: testpreempt.c 
 */
#include <8051.h>
#include "preemptive.h"

/* 
 * @@@ [2pt] 
 * declare your global variables here, for the shared buffer 
 * between the producer and consumer.  
 * Hint: you may want to manually designate the location for the 
 * variable.  you can use
 *        __data __at (0x30) type var; 
 * to declare a variable var of the type
 */ 
__data __at (0x26) char buffer[3];
__data __at (0x29) char head;
__data __at (0x2A) char tail;
__data __at (0x2B) char token;
__data __at (0x2C) char mutex;
__data __at (0x2D) char full;
__data __at (0x2E) char empty;
__data __at (0x3C) char token2;
__data __at (0x20) char producer1_ready;
__data __at (0x2F) char producer2_ready;  
#define L(x) LABEL(x)
#define LABEL(x) x##$

/* [8 pts] for this function
 * the producer in this test program generates one characters at a
 * time from 'A' to 'Z' and starts from 'A' again. The shared buffer
 * must be empty in order for the Producer to write.
 */
void Producer1(void) {
        /*
         * @@@ [2 pt]
         * initialize producer data structure, and then enter
         * an infinite loop (does not return)
         */
        token = 'A';
        while (1) {
                /* @@@ [6 pt]
                * wait for the buffer to be available, 
                * and then write the new data into the buffer */
                SemaphoreWaitBody(producer1_ready, L(__COUNTER__) );
                SemaphoreWaitBody(empty, L(__COUNTER__) );
                SemaphoreWaitBody(mutex, L(__COUNTER__) );
                __critical{
                buffer[tail] = token;
                if(tail!=2){
                        tail = tail + 1;
                }
                else{
                        tail = 0;
                }
                if(token == 'Z')
                        token = 'A';
                else
                        token = token + 1;
                }
                SemaphoreSignal(mutex);
                SemaphoreSignal(full);
                SemaphoreSignal(producer2_ready);
                            
        }
}
void Producer2(void) {
        /*
         * @@@ [2 pt]
         * initialize producer data structure, and then enter
         * an infinite loop (does not return)
         */
        token2 = '0';
        while (1) {
                        
                        /* @@@ [6 pt]
                        * wait for the buffer to be available, 
                        * and then write the new data into the buffer */
                        SemaphoreWaitBody(producer2_ready, L(__COUNTER__));
                        SemaphoreWaitBody(empty, L(__COUNTER__) );
                        SemaphoreWaitBody(mutex, L(__COUNTER__) );
                        __critical{
                        buffer[tail] = token2;
                        if(tail!=2){
                                tail = tail + 1;
                        }
                        else{
                                tail = 0;
                        }
                        if(token2 == '9')
                                token2 = '0';
                        else
                                token2 = token2 + 1;
                        }
                        SemaphoreSignal(mutex);
                        SemaphoreSignal(full);
                        SemaphoreSignal(producer1_ready);
        }
}

/* [10 pts for this function]
 * the consumer in this test program gets the next item from
 * the queue and consume it and writes it to the serial port.
 * The Consumer also does not return.
 */
void Consumer(void) {
        /* @@@ [2 pt] initialize Tx for polling */
        EA = 0;
        TMOD |= 0x20;
        TH1 = -6;
        SCON = 0x50;
        TR1 = 1;
        EA = 1;
        while (1) {
                /* @@@ [2 pt] wait for new data from producer
                 * @@@ [6 pt] write data to serial port Tx, 
                 * poll for Tx to finish writing (TI),
                 * then clear the flag
                 */
                SemaphoreWaitBody(full,  L(__COUNTER__));
                SemaphoreWaitBody(mutex,  L(__COUNTER__));
                __critical{
                    SBUF = buffer[head];
                    if(head!=2)
                        head = head + 1;
                    else
                        head = 0; 
                }
                SemaphoreSignal(mutex);
                SemaphoreSignal(empty);
                while(!TI){
                        continue;
                }
                TI = 0;
        }
}

/* [5 pts for this function]
 * main() is started by the thread bootstrapper as thread-0.
 * It can create more thread(s) as needed:
 * one thread can acts as producer and another as consumer.
 */
void main(void) {
          /* 
           * @@@ [1 pt] initialize globals
           *  
           * @@@ [4 pt] set up Producer and Consumer.
           * Because both are infinite loops, there is no loop
           * in this function and no return.
           */
          SemaphoreCreate(full, 0);
          SemaphoreCreate(mutex, 1);
          SemaphoreCreate(empty,3);
          SemaphoreCreate(producer1_ready, 1);
          SemaphoreCreate(producer2_ready, 0);
          head = 0;
          tail = 0;
          token = 'A';
          token2 = '0';
          ThreadCreate( Producer1 );
          ThreadCreate( Producer2 );
          Consumer();     
}

void _sdcc_gsinit_startup(void) {
        __asm
                ljmp  _Bootstrap
        __endasm;
}

void _mcs51_genRAMCLEAR(void) {}
void _mcs51_genXINIT(void) {}
void _mcs51_genXRAMCLEAR(void) {}

void timer0_ISR(void) __interrupt(1) {
        __asm
                ljmp _myTimer0Handler
        __endasm;
}

