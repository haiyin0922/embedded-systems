#include <msp430.h>
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

int main(void) {

    enum{ red1_on, red1_off, red2_on, red2_off, green_on, green_off, both_on, both_off };
    enum{ Timer0_A3_up, Button_down };

    WDTCTL = WDTPW | WDTHOLD;
    P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);
    P1REN |= B1;
    P1OUT |= B1;

    TA0CTL |= MC_1|TASSEL_1|TACLR;
    BCSCTL3 |= LFXT1S_2;

    TA1CTL |= MC_1|TASSEL_2|TACLR;
    DCOCTL |= CALDCO_1MHZ;
    BCSCTL2 |= DIVS_3;

    TA1CCR0 = 9999;

    volatile unsigned int state = red1_on;
    volatile unsigned int event = Timer0_A3_up;
    volatile unsigned int press = 0;

    for(;;){
        switch(state){
            case red1_on:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT ^= LED1;
                        P1OUT &= ~LED2;
                        TA0CCR0 = 5999;
                        state = red1_off;
                        break;
                    case Button_down:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = both_on;
                        break;
                }
                break;
            case red1_off:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT ^= LED1;
                        P1OUT &= ~LED2;
                        TA0CCR0 = 5999;
                        state = red2_on;
                        break;
                    case Button_down:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = both_on;
                        break;
                }
                break;
            case red2_on:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT ^= LED1;
                        P1OUT &= ~LED2;
                        TA0CCR0 = 5999;
                        state = red2_off;
                        break;
                    case Button_down:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = both_on;
                        break;
                }
                break;
            case red2_off:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT ^= LED1;
                        P1OUT &= ~LED2;
                        TA0CCR0 = 5999;
                        state = green_on;
                        break;
                    case Button_down:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = both_on;
                        break;
                }
                break;
            case green_on:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT ^= LED2;
                        P1OUT &= ~LED1;
                        TA0CCR0 = 10799;
                        state = green_off;
                        break;
                    case Button_down:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = both_on;
                        break;
                }
                break;
            case green_off:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT ^= LED2;
                        P1OUT &= ~LED1;
                        TA0CCR0 = 5999;
                        state = red1_on;
                        break;
                    case Button_down:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = both_on;
                        break;
                }
                break;
            case both_on:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = red1_on;
                        break;
                    case Button_down:
                        P1OUT ^= LED1 + LED2;
                        TA0CCR0 = 5999;
                        state = both_off;
                        break;
                }
                break;
            case both_off:
                switch(event){
                    case Timer0_A3_up:
                        P1OUT &= ~(LED1 + LED2);
                        TA0CCR0 = 5999;
                        state = red1_on;
                        break;
                    case Button_down:
                        P1OUT ^= LED1 + LED2;
                        TA0CCR0 = 5999;
                        state = both_on;
                        break;
                }
                break;
        }

        while (!(TA0CTL & TAIFG)) {
            if((P1IN & B1) != 0 && press > 0){
                press = 0;
            }
            if((P1IN & B1) == 0 && event == Timer0_A3_up && press < 26 && (TA1CTL & TAIFG)){
                press++;
                TA1CTL &= ~TAIFG;
            }
            else if(press == 26 && (TA1CTL & TAIFG)){
                press = 0;
                TA1CTL &= ~TAIFG;
                event = Button_down;
            }
            else if((P1IN & B1) != 0 && event == Button_down){
                event = Timer0_A3_up;
                break;
            }
        }
        TA0CTL &= ~TAIFG;
    }
    return 0;
}
