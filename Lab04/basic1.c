#include "msp430.h"
#define LED1 BIT0
#define LED2 BIT6

enum{ red1_on, red1_off, red2_on, red2_off, green_on, green_off, both_on, both_off };
volatile unsigned int state = red1_on;
volatile unsigned int flag = 0, pattern = 1, temp = 0;

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;
    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);
    ADC10CTL0 |= ENC;
    __enable_interrupt();

    TA0CCR0 = 12499;
    TA0CCTL1 = OUTMOD_3;
    TA0CCR1 = 12498;
    TA0CTL = MC_1 | TASSEL_2 | TACLR | ID_3;
    DCOCTL |= CALDCO_1MHZ;
    BCSCTL2 |= DIVS_3;

    TA1CCR0 = 5999;
    TA1CCTL0 |= CCIE;
    TA1CTL |= MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;

    for(;;){
        if(flag){
            flag = 0;
            TA1R = 0;
            switch(state){
                case red1_on:
                    P1OUT ^= LED1;
                    TA1CCR0 = 5999;
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

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    temp = ADC10MEM;
    if(ADC10MEM < 746 && pattern == 2){
        pattern = 1;
        P1OUT &= ~(LED1 + LED2);
        state = red1_on;
    }
    else if(ADC10MEM >= 746 && pattern == 1){
        pattern = 2;
        P1OUT &= ~(LED1 + LED2);
        state = both_on;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    flag = 1;
}
