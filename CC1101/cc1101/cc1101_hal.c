#include "cc1101_hal.h"

void cc1101_hal_delay_cycles(uint32_t cycles)
{
    while (cycles--) __no_operation();
}

void cc1101_hal_delay_ms(uint16_t ms)
{
    uint32_t cycles = (SMCLK_FREQUENCY_HZ / 1000UL) * (uint32_t)ms;
    cc1101_hal_delay_cycles(cycles);
}

void cc1101_hal_csn_low(void)
{
    CC1101_CSN_PORT_OUT &= ~CC1101_CSN_PIN;
}

void cc1101_hal_csn_high(void)
{
    CC1101_CSN_PORT_OUT |= CC1101_CSN_PIN;
}

void cc1101_hal_spi_init(void)
{
    // Hold eUSCI_B1 in reset while we configure it //
    // UCB1SIMO(4) -> P4.4
    // UCB1SOMI(4) -> P4.3
    // UCB1CLK(4)  -> P5.3
    // UCB1STE(4)  -> P5.4
    UCB1CTLW0 = UCSWRST | UCMST | UCSYNC | UCMSB | UCCKPH;   //use CC1101’s SPI phase
    SYSCFG3 |= USCIB1RMP;

    /* --- Configure SPI pins as peripheral function (UCB1) --- */
    // MOSI P4.4
    P4DIR  |= BIT4;          // output
    P4SEL0 |= BIT4;          // SEL0 = 1
    P4SEL1 &= ~BIT4;         // SEL1 = 0

    // MISO P4.3
    P4DIR  &= ~BIT3;         // input
    P4SEL0 |= BIT3;          // SEL0 = 1
    P4SEL1 &= ~BIT3;         // SEL1 = 0

    // SCLK P5.3
    P5DIR  |= BIT3;          // output
    P5SEL0 |= BIT3;          // SEL0 = 1
    P5SEL1 &= ~BIT3;         // SEL1 = 0

    /* --- Configure UCB1 for 3-pin SPI master --- */
    UCB1CTLW0 = UCSWRST        // keep in reset
              | UCMST          // master mode
              | UCSYNC         // synchronous (SPI)
              | UCMSB;         // MSB first, mode 0 (CKPH=0, CKPL=0)

    UCB1CTLW0 |= UCSSEL__SMCLK; // SMCLK as clock source
    UCB1BRW = 4;                // SCLK = SMCLK / 4

    /* Release eUSCI_B1 for operation */
    UCB1CTLW0 &= ~UCSWRST;
}

void cc1101_hal_init(void)
{
    P5SEL0 &= ~BIT4;
    P5SEL1 &= ~BIT4;
    CC1101_CSN_PORT_DIR |= CC1101_CSN_PIN;
    cc1101_hal_csn_high();

    CC1101_GDO0_PORT_DIR &= ~CC1101_GDO0_PIN;
    CC1101_GDO0_PORT_REN |= CC1101_GDO0_PIN;
    CC1101_GDO0_PORT_OUT &= ~CC1101_GDO0_PIN;

    cc1101_hal_spi_init();
}

uint8_t cc1101_hal_spi_tx_rx(uint8_t data)
{
    while (!(UCB1IFG & UCTXIFG0));
    UCB1TXBUF = data;
    while (!(UCB1IFG & UCRXIFG0));
    return UCB1RXBUF;
}
