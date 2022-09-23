#include "msp430.h"
#include "stdio.h"
#define UART_TXD 0x02 // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD 0x04 // RXD on P1.2 (Timer0_A.CCI1A)
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)
#define LED1 BIT0
#define LED2 BIT6

unsigned int txData;  // UART internal TX variable
unsigned char rxBuffer; // Received UART character
enum{ red_on, red_off, green_on, green_off };
volatile unsigned int emergency = 0, flag = 0, state = green_on, aver = 0, cnt = 0, idx = 0;
unsigned char s[3], input[4];
int Temp[64];

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
void flash(char id, int on, int off);
void temp(int interval, int times);

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer
    DCOCTL = 0x00;             // Set DCOCLK to 1MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    P1OUT = 0x00;       // Initialize all GPIO
    P1SEL = UART_TXD + UART_RXD; // Use TXD/RXD pins
    P1DIR = 0xFF & ~UART_RXD; // Set pins to output

    P1DIR |= LED1 + LED2;
    P1OUT &= ~(LED1 + LED2);

    ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    //ADC10AE0 |= 0x02;    // P1.1 ADC10 option select
    ADC10DTC1 = 2;     // # of transfers
    ADC10SA = (int)Temp; // Buffer starting address
    // Enter LPM3 w/ interrupts

    __enable_interrupt();

    TA1CCR0 = 10799;
    TA1CCTL0 |= CCIE;
    TA1CTL |= MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;

    temp(1000, 0);     // Start Timer_A UART

    for (;;) {
        aver = (Temp[0] + Temp[1]) / 2;
        if(flag){
            flag = 0;
            if(emergency){
                __bis_SR_register(LPM0_bits);
               // Echo received character
                cnt++;
                if(cnt == 8){
                    cnt = 0;
                    //TimerA_UART_init();
                    sprintf(s, "%d", aver);
                    TimerA_UART_print(s);
                }
                TA1R = 0;
                switch(state){
                    case red_on:
                        P1OUT ^= LED1;
                        TA1CCR0 = 2399;
                        state = red_off;
                        break;
                    case red_off:
                        P1OUT ^= LED1;
                        TA1CCR0 = 3599;
                        state = red_on;
                        break;
                }
            }
            else{
                flash(1, 900, 500);
                if(aver > 745){
                    emergency = 1;
                    TimerA_UART_init();
                    TimerA_UART_print("Emergency!");
                    //temp(2000, 0);
                    P1OUT &= ~(LED1 + LED2);
                    state = red_on;
                }
            }
        }
    }
}

void flash(char id, int on, int off){
    TA1R = 0;
    switch(state){
        case green_on:
            P1OUT ^= LED2;
            TA1CCR0 = 12*on - 1;
            state = green_off;
            break;
        case green_off:
            P1OUT ^= LED2;
            TA1CCR0 = 12*off - 1;
            state = green_on;
            break;
    }
    _BIS_SR(LPM3_bits + GIE);
}

void temp(int interval, int times){
    TA0CCTL0 = CCIE;
    TA0CCTL1 = OUTMOD_3;
    TA0CTL = TASSEL_1 + MC_1;
    ADC10CTL0 |= ENC;
    TA0CCR0 = 12*interval - 1;
    TA0CCR1 = 12*interval - 2;
}

void TimerA_UART_init(void) {
    TA0CCTL0 = OUT;   // Set TXD idle as '1'
    TA0CCTL1 = SCS + CM1 + CAP + CCIE; // CCIS1 = 0
    // Set RXD: sync, neg edge, capture, interrupt
    TA0CTL = TASSEL_2 + MC_2; // SMCLK, continuous mode
}

void TimerA_UART_print(char *string) {
    while (*string) TimerA_UART_tx(*string++);
}

void TimerA_UART_tx(unsigned char byte) {
    while (TA0CCTL0 & CCIE); // Ensure last char TX'd
    TA0CCR0 = TA0R;      // Current count of TA counter
    TA0CCR0 += UART_TBIT; // One bit time till 1st bit
    TA0CCTL0 = OUTMOD0 + CCIE; // Set TXD on EQU0, Int
    txData = byte;       // Load char to be TXD
    txData |= 0x100;    // Add stop bit to TXData
    txData <<= 1;       // Add start bit
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    ADC10SA = (int)Temp;
}

#pragma vector = TIMER0_A0_VECTOR  // TXD interrupt
__interrupt void Timer_A0_ISR(void) {
    //if(emergency){
        static unsigned char txBitCnt = 10;
        TA0CCR0 += UART_TBIT; // Set TA0CCR0 for next intrp
        if (txBitCnt == 0) {  // All bits TXed?
            TA0CCTL0 &= ~CCIE;  // Yes, disable intrpt
            txBitCnt = 10;      // Re-load bit counter
        }
        else {
            if (txData & 0x01) {// Check next bit to TX
                TA0CCTL0 &= ~OUTMOD2; // TX '1¡¦ by OUTMODE0/OUT
            }
            else {
                TA0CCTL0 |= OUTMOD2;
            } // TX '0¡¥
            txData >>= 1;
            txBitCnt--;
        }
    //}
    TA0CCTL0 &= ~CCIFG;
}

#pragma vector = TIMER0_A1_VECTOR // RXD interrupt
__interrupt void Timer_A1_ISR(void) {
    //if(emergency){
        static unsigned char rxBitCnt = 8;
        static unsigned char rxData = 0;
        switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {
        case TA0IV_TACCR1:     // TACCR1 - UART RXD
            TA0CCR1 += UART_TBIT;// Set TACCR1 for next int
            if (TA0CCTL1 & CAP) { // On start bit edge
                TA0CCTL1 &= ~CAP;   // Switch to compare mode
                TA0CCR1 += UART_TBIT_DIV_2;// To middle of D0
            }
            else {             // Get next data bit
                rxData >>= 1;

                if (TA0CCTL1 & SCCI) { // Get bit from latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {  // All bits RXed?
                    if(rxData == 13){
                        idx = 0;
                        if(input[0] == 'A' && input[1] == 'c' && input[2] == 'k' && input[3] == '!'){
                            emergency = 0;
                            P1OUT &= ~(LED1 + LED2);
                            state = green_on;
                            Temp[0] = 0;
                            Temp[1] = 0;
                            temp(1000, 0);
                        }
                    }
                    else{
                        input[idx] = rxData;
                        idx++;
                    }
                    if(idx>3) idx = 0;
                    rxBuffer = rxData;  // Store in global
                    rxBitCnt = 8;       // Re-load bit counter
                    TA0CCTL1 |= CAP;     // Switch to capture
                    __bic_SR_register_on_exit(LPM0_bits);
                  // Clear LPM0 bits SR
                }
            }
            break;
        }
    //}
    TA0CCTL1 &= ~CCIFG;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    flag = 1;
    _BIC_SR(LPM3_EXIT);
    TA1CCTL0 &= ~CCIFG;
}

