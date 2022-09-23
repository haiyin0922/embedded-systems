#include <msp430.h> // Specific device
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

enum{ red1_on, red1_off, red2_on, red2_off, green_on, green_off, both };
volatile unsigned int state = red1_on;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    TA0CCR0 = 5999; // Upper limit of count for TA0R
    TA0CCTL0 = CCIE; // Enable interrupts
    TA0CTL = MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;

    P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);
    P1REN |= B1;                   // Enable internal pull-up/down resistors
    P1OUT |= B1;                   //Select pull-up mode for P1.3
    P1IES |= B1;                       // P1.3 interrupt enabled
    P1IFG &= ~B1;
    P1IE |= B1;
    // Up mode, divide clock by 8, clock from SMCLK, clear
    __enable_interrupt(); // Enable interrupts (intrinsic)
    for (;;) { } // Loop forever doing nothing
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    if(state != both){
        state = both;
    }
    else{
        state = red1_on;
    }
    P1OUT &= ~(LED1 + LED2);
    P1IFG &= ~B1;
    P1IES ^= B1;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
    TA0R = 0;
    switch(state){
        case red1_on:
            P1OUT ^= LED1;
            TA0CCR0 = 5999;
            state = red1_off;
            break;
        case red1_off:
            P1OUT ^= LED1;
            state = red2_on;
            break;
        case red2_on:
            P1OUT ^= LED1;
            state = red2_off;
            break;
        case red2_off:
            P1OUT ^= LED1;
            state = green_on;
            break;
        case green_on:
            P1OUT ^= LED2;
            TA0CCR0 = 10799;
            state = green_off;
            break;
        case green_off:
            P1OUT ^= LED2;
            TA0CCR0 = 5999;
            state = red1_on;
            break;
        case both:
            P1OUT ^= LED1 + LED2;
            TA0CCR0 = 5999;
            state = both;
            break;
    }
}
