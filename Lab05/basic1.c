#include <msp430.h> 
#define LED1 BIT0
#define LED2 BIT6

enum{ red1_on, red1_off, red2_on, red2_off, green_on, green_off, both_on, both_off };
volatile unsigned int state = red1_on;
volatile unsigned int flag = 0, emergency = 0, pattern = 1;
volatile unsigned int celsius = 0, cnt0 = 0, cnt3 = 0;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);
	
	ADC10CTL1 = SHS_1 + CONSEQ_0 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
    ADC10CTL0 |= ENC;
    __enable_interrupt();

	TA0CCR0 = 9599;
	TA0CCTL1 = OUTMOD_3;
	TA0CCR1 = 9598;
    TA0CTL |= MC_1 | TASSEL_1 | TACLR;

    TA1CCR0 = 5999;
    TA1CCTL0 |= CCIE;
    TA1CTL |= MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;

    for(;;){
        if(pattern == 1) _BIS_SR(LPM3_bits + GIE);
        else _BIS_SR(LPM0_bits + GIE);
        if(flag){
            flag = 0;
            TA1R = 0;
            switch(state){
                case red1_on:
                    P1OUT ^= LED1;
                    if(pattern == 1) TA1CCR0 = 5999;
                    else TA1CCR0 = 3599;
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
	return 0;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    celsius = (ADC10MEM * 1.5 / 1023 - 0.986) / 0.00355;
    if(celsius > 28){
        pattern = 2;
        TA0CCR0 = 4799;
        TA0CCR1 = 4798;
    }
    else{
        pattern = 1;
        TA0CCR0 = 9599;
        TA0CCR1 = 9598;
    }
    ADC10CTL0 |= ADC10SC;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    if(pattern == 1){
        cnt3++;
        _BIC_SR(LPM3_EXIT);
    }
    else{
        cnt0++;
        _BIC_SR(LPM0_EXIT);
    }
    flag = 1;
}

