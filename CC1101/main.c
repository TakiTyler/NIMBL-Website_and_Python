#include <driverlib.h>
#include "cc1101/cc1101.h"
#include "cc1101/cc1101_hal.h"
#include "cc1101/cc1101_regs.h"  // make sure IOCFG0 is defined

int main(void) {
    // volatile uint32_t n;
    WDTCTL = WDTPW | WDTHOLD;      // Stop watchdog

    // Disable the GPIO high-impedance mode
    PM5CTL0 &= ~LOCKLPM5;

    // --- Debug LED on P1.0 ---
    P1SEL0 &= ~BIT0;
    P1SEL1 &= ~BIT0;
    P1DIR  |=  BIT0;
    P1OUT  &= ~BIT0;               // start off

    // --- Initialize CC1101 ---
    cc1101_init();                 // if this hangs, you won’t see the next blinks
    
    // --- Blink after initializing CC1101
    P1OUT ^= BIT0;                 // ON
    __delay_cycles(1000000);       // visible delay
    P1OUT ^= BIT0;                 // OFF
    __delay_cycles(1000000);

    // --- Read CC1101 PARTNUM / VERSION for debugging ---
    volatile uint8_t part   = cc1101_read_status(CC1101_PARTNUM);
    volatile uint8_t ver    = cc1101_read_status(CC1101_VERSION);
    volatile uint8_t freq2  = cc1101_read_reg(CC1101_FREQ2);
    volatile uint8_t mdmcfg4 = cc1101_read_reg(CC1101_MDMCFG4);
    volatile uint8_t pktlen = cc1101_read_reg(CC1101_PKTLEN);
    // --- Blink TWICE to indicate cc1101_init() returned ---
    // for (n = 0; n < 2; n++)
    // {
    //     P1OUT ^= BIT0;             // ON
    //     __delay_cycles(1000000);
    //     P1OUT ^= BIT0;             // OFF
    //     __delay_cycles(1000000);
    // }

    // --- Optional: send a tiny test packet ---
    uint8_t test_data[] = { 'H', 'i' };

    //**************************************************************************************//

    //****************************BLINKING LED******************************************//
    // volatile uint32_t i;

    // // Stop watchdog timer
    // WDT_A_hold(WDT_A_BASE);

    // // Set P1.0 to output direction
    // GPIO_setAsOutputPin(
    //     GPIO_PORT_P1,
    //     GPIO_PIN0
    //     );

    // // Disable the GPIO power-on default high-impedance mode
    // // to activate previously configured port settings
    // PMM_unlockLPM5();

    // while(1)
    // {
    //     // Toggle P1.0 output
    //     GPIO_toggleOutputOnPin(
    //         GPIO_PORT_P1,
    //         GPIO_PIN0
    //         );

    //     // Delay
    //     for(i=10000; i>0; i--);
    // }
    //*************************************************************************************//
}
