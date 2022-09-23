#include <msp430.h> // Specific device
#include <intrinsics.h> // Intrinsic functions
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

enum{ red1_on, red1_off, red2_on, red2_off, green_on, green_off, both };
volatile unsigned int state = red1_on;
volatile unsigned int press = 0, cnt = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    TA0CCR0 = 5999; // Upper limit of count for TA0R
    TA0CCTL0 |= CCIE; // Enable interrupts
    TA0CTL |= MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;


    TA1CCR0 = 4999;
    TA1CCTL0 |= CCIE;
    TA1CTL |= MC_1 | TASSEL_2 | TACLR;
    DCOCTL |= CALDCO_1MHZ;
    BCSCTL2 |= DIVS_3;


    P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);
    P1REN |= B1;                   // Enable internal pull-up/down resistors
    P1OUT |= B1;                   //Select pull-up mode for P1.3
    P1IES |= B1;                       // P1.3 interrupt enabled
    P1IFG &= ~B1;
    P1IE |= B1;

    __enable_interrupt(); // Enable interrupts (intrinsic)
    for (;;) { } // Loop forever doing nothing
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    press = !press;
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

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    TA1R = 0;
    if(press && state != both){
        cnt++;
    }
    else if(!press && cnt >= 75 && state != both){
        P1OUT &= ~(LED1 + LED2);
        state = both;
    }
    else if(state == both && cnt > 0){
        cnt--;
    }
    else if(state == both && cnt == 0){
        P1OUT &= ~(LED1 + LED2);
        state = red1_on;
    }
}
