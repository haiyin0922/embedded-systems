#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    P1DIR |= LED1 + LED2;
    TA0CTL |= MC_1|TASSEL_1|TACLR;
    BCSCTL3 |= LFXT1S_2;

    volatile unsigned int cnt = 0;
    for(;;) {
        cnt++;
        if(cnt == 6){
            TA0CCR0 = 5999;
            P1OUT &= ~(LED1 + LED2);
            cnt = 0;
        }
        else if(cnt == 5){
            TA0CCR0 = 10799;
            P1OUT &= ~LED1;
            P1OUT ^= LED2;
        }
        else{
            TA0CCR0 = 5999;
            P1OUT &= ~LED2;
            P1OUT ^= LED1;
        }
        while (!(TA0CTL & TAIFG)) {} // Wait for time up
        TA0CTL &= ~TAIFG;  // Clear overflow flag
    }
    return 0;
}
