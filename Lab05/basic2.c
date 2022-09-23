#include <msp430.h> 
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

enum{ red1_on, red1_off, red2_on, red2_off, green_on, green_off, both_on, both_off };
volatile unsigned int state = red1_on;
volatile unsigned int flag = 0, click = 0, measure = 0;
volatile unsigned int celsius = 0, cnt0 = 0, cnt3 = 0;
volatile unsigned int temp = 0;

int adc[4]; // Buffer to store the ADC values

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);
    P1REN |= B1;                   // Enable internal pull-up/down resistors
    P1OUT |= B1;                   //Select pull-up mode for P1.3
    P1IES |= B1;                       // P1.3 interrupt enabled
    P1IFG &= ~B1;
    P1IE |= B1;

    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10DTC1 = 4;     // # of transfers
    ADC10SA = (int)adc; // Buffer starting address
    __enable_interrupt();

    TA0CCTL1 = OUTMOD_3;
    TA0CCR1 = 2998;

    TA0CCR0 = 5999;
    TA0CCTL0 |= CCIE;
    TA0CTL |= MC_1 | TASSEL_1 | TACLR;

    TA1CCR0 = 5999;
    TA1CCTL0 |= CCIE;
    TA1CTL |= MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;

    for(;;){
        if(measure) _BIS_SR(LPM0_bits + GIE);
        else _BIS_SR(LPM3_bits + GIE);
        temp = TA0CCR0;
        if(measure && flag){
            flag = 0;
            TA1R = 0;
            switch(state){
                case both_on:
                    if(celsius < 28) P1OUT ^= LED1 + LED2;
                    else P1OUT ^= LED1;
                    TA1CCR0 = 2399;
                    state = both_off;
                    break;
                case both_off:
                    P1OUT &= ~(LED1 + LED2);
                    TA1CCR0 = 3599;
                    state = both_on;
                    break;
            }

        }
        else if(!measure && flag){
            flag = 0;
            TA0R = 0;
            switch(state){
                case red1_on:
                    P1OUT ^= LED1;
                    TA0CCR0 = 5999;
                    state = red1_off;
                    break;
                case red1_off:
                    P1OUT ^= LED1;
                    TA0CCR0 = 5999;
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
            }
        }
    }
    return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    click++;
    if(click == 1){
        TA1R = 0;
    }
    else if(click == 2){
        click = 0;
        measure = !measure;
        P1OUT &= ~(LED1 + LED2);
        if(measure){
            ADC10CTL0 |= ENC;
            TA0CCR0 = 2999;
            state = both_on;
        }
        else{
            ADC10CTL0 &= ~ENC;
            TA0CCR0 = 5999;
            state = red1_on;
        }
    }
    P1IFG &= ~B1;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    ADC10SA = (int)adc;
    celsius = ((adc[0]+adc[1]+adc[2]+adc[3])/4 * 1.5 / 1023 - 0.986) / 0.00355;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    if(measure){
        cnt0++;
        _BIC_SR(LPM0_EXIT);
    }
    else{
        cnt3++;
        _BIC_SR(LPM3_EXIT);
        flag = 1;
    }
    TA0CCTL0 &= ~CCIFG;
    TA0CCTL1 &= ~CCIFG;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    if(measure){
        cnt0++;
        _BIC_SR(LPM0_EXIT);
        flag = 1;
    }
    else{
        cnt3++;
        _BIC_SR(LPM3_EXIT);
    }
    click = 0;
    TA1CCTL0 &= ~CCIFG;
}

