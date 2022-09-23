#include "msp430.h"
#include "stdio.h"
#define UART_TXD 0x02 // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD 0x04 // RXD on P1.2 (Timer0_A.CCI1A)
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)
#define B1 BIT3
unsigned int txData;  // UART internal TX variable
unsigned char rxBuffer; // Received UART character
volatile unsigned int click = 0, enhanced = 0, num = 0, flag = 0;
unsigned char s[4];

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer
    DCOCTL = 0x00;             // Set DCOCLK to 1MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    P1OUT = 0x00;       // Initialize all GPIO
    P1SEL = UART_TXD + UART_RXD; // Use TXD/RXD pins
    P1DIR = 0xFF & ~UART_RXD; // Set pins to output

    P1DIR &= ~B1;
    P1REN |= B1;                   // Enable internal pull-up/down resistors
    P1OUT |= B1;                   //Select pull-up mode for P1.3
    P1IES |= B1;                   // P1.3 interrupt enabled
    P1IFG &= ~B1;
    P1IE |= B1;
    __enable_interrupt();

    TA1CCR0 = 35999;
    TA1CCTL0 |= CCIE;
    TA1CTL |= MC_1 | TASSEL_1 | TACLR;
    BCSCTL3 |= LFXT1S_2;

    TimerA_UART_init();     // Start Timer_A UART
    //TimerA_UART_print("G2xx3 TimerA UART\r\n");
    //TimerA_UART_print("READY.\r\n");
    for (;;) {
        if(flag){
            flag = 0;
            __bis_SR_register(LPM0_bits);
            TimerA_UART_print("HELLO!\r\n");
        }
        else if(!enhanced){
            __bis_SR_register(LPM0_bits);
            // Echo received character
            if(rxBuffer == 13){
                sprintf(s, "#%d ", num);
                TimerA_UART_print(s);
                num = 0;
            }
            else{
                num++;
                // Echo received character
                TimerA_UART_tx(rxBuffer);
            }
        }
    }
}
void TimerA_UART_print(char *string) {
    while (*string) TimerA_UART_tx(*string++);
}

void TimerA_UART_init(void) {
    TA0CCTL0 = OUT;   // Set TXD idle as '1'
    TA0CCTL1 = SCS + CM1 + CAP + CCIE; // CCIS1 = 0
    // Set RXD: sync, neg edge, capture, interrupt
    TA0CTL = TASSEL_2 + MC_2; // SMCLK, continuous mode
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

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    click++;
    if(click == 2){
        click = 0;
        enhanced = !enhanced;
    }
    P1IFG &= ~B1;
    P1IES ^= B1;
}

#pragma vector = TIMER0_A0_VECTOR  // TXD interrupt
__interrupt void Timer_A0_ISR(void) {
    static unsigned char txBitCnt = 10;
    TA0CCR0 += UART_TBIT; // Set TA0CCR0 for next intrp
    if (txBitCnt == 0) {  // All bits TXed?
        TA0CCTL0 &= ~CCIE;  // Yes, disable intrpt
        txBitCnt = 10;      // Re-load bit counter
    }
    else {
        if (txData & 0x01) {// Check next bit to TX
            TA0CCTL0 &= ~OUTMOD2; // TX '1�� by OUTMODE0/OUT
        }
        else {
            TA0CCTL0 |= OUTMOD2;
        } // TX '0��
        txData >>= 1;
        txBitCnt--;
    }
}

#pragma vector = TIMER0_A1_VECTOR // RXD interrupt
__interrupt void Timer_A1_ISR(void) {
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
                rxBuffer = rxData;  // Store in global
                rxBitCnt = 8;       // Re-load bit counter
                TA0CCTL1 |= CAP;     // Switch to capture
                __bic_SR_register_on_exit(LPM0_bits);
              // Clear LPM0 bits SR
            }
        }
        break;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void) {
    if(enhanced){
        __bic_SR_register_on_exit(LPM0_bits);
        flag = 1;
    }
}

