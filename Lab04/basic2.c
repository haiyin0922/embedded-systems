#include "msp430.h"
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

enum{ red1_on, red1_off, red2_on, red2_off, green_on, green_off, both_on, both_off };
volatile unsigned int state = red1_on;
volatile unsigned int flag = 0, press = 0, measure = 0, pattern = 1;
volatile unsigned int celsius = 0;

int adc[4]; // Buffer to store the ADC values

void main(void) {
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10DTC1 = 4;     // # of transfers
    ADC10SA = (int)adc; // Buffer starting address
    ADC10CTL0 |= ENC;

    P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);
    P1REN |= B1;                   // Enable internal pull-up/down resistors
    P1OUT |= B1;                   //Select pull-up mode for P1.3
    P1IES |= B1;                       // P1.3 interrupt enabled
    P1IFG &= ~B1;
    P1IE |= B1;

    __enable_interrupt(); // Enable interrupts

    TA0CCR0 = 59999;
    TA0CCTL0 |= CCIE;
    TA0CTL |= MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;

    TA1CCR0 = 5999;
    TA1CCTL0 |= CCIE;
    TA1CTL |= MC_1 | TASSEL_1 | TACLR;

    for(;;){
        if(flag){
            flag = 0;
            TA1R = 0;
            switch(state){
                case red1_on:
                    P1OUT ^= LED1;
                    if(pattern == 1) TA1CCR0 = 5999;
                    else TA1CCR0 = 2399;
                    state = red1_off;
                    break;
                case red1_off:
                    P1OUT ^= LED1;
                    if(pattern == 1){
                        TA1CCR0 = 5999;
                        state = red2_on;
                    }
                    else{
                        TA1CCR0 = 2399;
                        state = red1_on;
                    }
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
                    TA1CCR0 = 10799;
                    state = green_off;
                    break;
                case green_off:
                    P1OUT ^= LED2;
                    TA1CCR0 = 5999;
                    state = red1_on;
                    break;
                case both_on:
                    P1OUT ^= LED1 + LED2;
                    TA1CCR0 = 4199;
                    state = both_off;
                    break;
                case both_off:
                    P1OUT ^= LED1 + LED2;
                    TA1CCR0 = 7799;
                    state = both_on;
                    break;
            }
        }
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    press = !press;
    if(press) TA0R = 0;
    else if(!press && measure){
        measure = 0;
        pattern = 1;
        P1OUT &= ~(LED1 + LED2);
        state = red1_on;
        TA0R = 0;
        TA0CCR0 = 59999;
        TA0CCTL1 &= ~CCIFG;
    }
    P1IFG &= ~B1;
    P1IES ^= B1;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    ADC10SA = (int)adc;
    celsius = ((adc[0]+adc[1]+adc[2]+adc[3])/4 * 1.5 / 1023 - 0.986) / 0.00355;
    if(measure && (adc[0]+adc[1]+adc[2]+adc[3])/4 >= 746 && pattern != 2){
        pattern = 2;
        P1OUT &= ~(LED1 + LED2);
        state = both_on;
    }
    else if(measure && (adc[0]+adc[1]+adc[2]+adc[3])/4 < 746 && pattern != 3){
        pattern = 3;
        P1OUT &= ~(LED1 + LED2);
        state = red1_on;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    flag = 1;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    if(press && !measure){
        measure = 1;
        TA0CCR0 = 2399;
        TA0CCTL1 = OUTMOD_3;
        TA0CCR1 = 2398;
        TA0R = 2397;
    }
}

