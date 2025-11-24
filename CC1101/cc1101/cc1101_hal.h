#ifndef CC1101_HAL_H
#define CC1101_HAL_H

#include <stdint.h>
#include <msp430.h>

/*
 * CC1101 HAL for MSP430FR2476 using UCB1 SPI:
 * CSn  -> P5.4
 * SCLK -> P5.3
 * SOMI -> P4.3
 * SIMO -> P4.4
 */

#define CC1101_CSN_PORT_DIR   P5DIR
#define CC1101_CSN_PORT_OUT   P5OUT
#define CC1101_CSN_PIN        BIT4

#define CC1101_GDO0_PORT_DIR  P1DIR
#define CC1101_GDO0_PORT_IN   P1IN
#define CC1101_GDO0_PORT_OUT  P1OUT
#define CC1101_GDO0_PORT_REN  P1REN
#define CC1101_GDO0_PIN       BIT2

#define CC1101_SPI_MOSI_DIR   P4DIR
#define CC1101_SPI_MOSI_SEL0  P4SEL0
#define CC1101_SPI_MOSI_SEL1  P4SEL1
#define CC1101_SPI_MOSI_PIN   BIT4

#define CC1101_SPI_MISO_DIR   P4DIR
#define CC1101_SPI_MISO_SEL0  P4SEL0
#define CC1101_SPI_MISO_SEL1  P4SEL1
#define CC1101_SPI_MISO_PIN   BIT3

#define CC1101_SPI_SCLK_DIR   P5DIR
#define CC1101_SPI_SCLK_SEL0  P5SEL0
#define CC1101_SPI_SCLK_SEL1  P5SEL1
#define CC1101_SPI_SCLK_PIN   BIT3

#define CC1101_SPI_MISO_PORT_IN  P4IN

#ifndef SMCLK_FREQUENCY_HZ
#define SMCLK_FREQUENCY_HZ 8000000UL
#endif

#ifdef __cplusplus
extern "C" {
#endif

void cc1101_hal_init(void);
void cc1101_hal_spi_init(void);
uint8_t cc1101_hal_spi_tx_rx(uint8_t data);
void cc1101_hal_csn_low(void);
void cc1101_hal_csn_high(void);
void cc1101_hal_delay_cycles(uint32_t cycles);
void cc1101_hal_delay_ms(uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif
