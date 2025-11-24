#include "cc1101.h"
#include "cc1101_hal.h"
#include "cc1101_config.h"
/* Internal helpers */
static void cc1101_select(void)
{
    cc1101_hal_csn_low();

    /* Wait until MISO goes low (chip ready). Optional but recommended. */
    while (CC1101_SPI_MISO_PORT_IN & CC1101_SPI_MISO_PIN);
}

static void cc1101_deselect(void)
{
    cc1101_hal_csn_high();
}

void cc1101_write_reg(uint8_t addr, uint8_t value)
{
    cc1101_select();
    cc1101_hal_spi_tx_rx(addr | CC1101_WRITE_SINGLE);
    cc1101_hal_spi_tx_rx(value);
    cc1101_deselect();
}

uint8_t cc1101_read_reg(uint8_t addr)
{
    uint8_t val;

    cc1101_select();
    cc1101_hal_spi_tx_rx(addr | CC1101_READ_SINGLE);
    val = cc1101_hal_spi_tx_rx(0xFF);
    cc1101_deselect();

    return val;
}

void cc1101_write_burst(uint8_t addr, const uint8_t *buffer, uint8_t length)
{
    uint8_t i;

    cc1101_select();
    cc1101_hal_spi_tx_rx(addr | CC1101_WRITE_BURST);
    for (i = 0; i < length; i++) {
        cc1101_hal_spi_tx_rx(buffer[i]);
    }
    cc1101_deselect();
}

void cc1101_read_burst(uint8_t addr, uint8_t *buffer, uint8_t length)
{
    uint8_t i;

    cc1101_select();
    cc1101_hal_spi_tx_rx(addr | CC1101_READ_BURST);
    for (i = 0; i < length; i++) {
        buffer[i] = cc1101_hal_spi_tx_rx(0xFF);
    }
    cc1101_deselect();
}

uint8_t cc1101_read_status(uint8_t addr)
{
    uint8_t val;

    cc1101_select();
    cc1101_hal_spi_tx_rx(addr | CC1101_READ_BURST);
    val = cc1101_hal_spi_tx_rx(0xFF);
    cc1101_deselect();

    return val;
}

void cc1101_strobe(uint8_t strobe)
{
    cc1101_select();
    cc1101_hal_spi_tx_rx(strobe);
    cc1101_deselect();
}

void cc1101_apply_config()
{
    uint8_t i;
    // Write all configuration registers from the table
    for (i = 0; i < cc1101_default_settings_count; i++) {
        cc1101_write_reg(cc1101_default_settings[i].addr,
                         cc1101_default_settings[i].value);
    }

    // Write PA table (power level settings)
    cc1101_write_burst(CC1101_PATABLE,
                       cc1101_default_patable,
                       CC1101_PATABLE_SIZE);
}

void cc1101_reset(void)
{
    /* CSn high, then low, then high as per datasheet */
    cc1101_hal_csn_high();
    cc1101_hal_delay_ms(1);
    cc1101_hal_csn_low();
    cc1101_hal_delay_ms(1);
    cc1101_hal_csn_high();
    cc1101_hal_delay_ms(1);

    /* Send SRES strobe */
    cc1101_select();
    cc1101_hal_spi_tx_rx(CC1101_SRES);
    cc1101_deselect();

    /* Wait for reset to complete */
    cc1101_hal_delay_ms(1);
}

void cc1101_init(void)
{
    cc1101_hal_init();
    cc1101_reset();
    cc1101_apply_config();
}

/*
 * Blocking transmit of a single packet.
 * Returns 0 on success, non-zero on error.
 */
uint8_t cc1101_send_packet(const uint8_t *data, uint8_t length)
{
    uint8_t txbytes;
    uint8_t i;
    if (length == 0 || length > 61) {
        return 1; /* invalid length */
    }

    /* Flush TX FIFO */
    cc1101_strobe(CC1101_SFTX);

    /* Write length byte followed by payload */
    cc1101_select();
    cc1101_hal_spi_tx_rx(CC1101_TXFIFO | CC1101_WRITE_BURST);
    cc1101_hal_spi_tx_rx(length);

    for (i = 0; i < length; i++) {
        cc1101_hal_spi_tx_rx(data[i]);
    }
    cc1101_deselect();

    /* Strobe TX */
    cc1101_strobe(CC1101_STX);

    /* Wait until TX FIFO is empty */
    do {
        txbytes = cc1101_read_status(CC1101_TXBYTES) & 0x7F;
    } while (txbytes != 0);

    /* Enter IDLE */
    cc1101_strobe(CC1101_SIDLE);

    return 0;
}

/*
 * Blocking receive of a single packet with simple timeout.
 *  - On entry, *length must contain size of buffer.
 *  - On exit, *length is set to actual packet length.
 * Returns 0 on success, non-zero on error or timeout.
 */
uint8_t cc1101_receive_packet(uint8_t *data, uint8_t *length, uint16_t timeout_ms)
{
    uint16_t elapsed = 0;
    uint8_t rxbytes;
    uint8_t pkt_len;

    if (!data || !length || *length == 0) {
        return 1;
    }

    /* Flush RX FIFO and enter RX */
    cc1101_strobe(CC1101_SFRX);
    cc1101_strobe(CC1101_SRX);

    /* Wait for data in RX FIFO or timeout */
    while (elapsed < timeout_ms) {
        rxbytes = cc1101_read_status(CC1101_RXBYTES) & 0x7F;
        if (rxbytes > 0) {
            break;
        }
        cc1101_hal_delay_ms(1);
        elapsed++;
    }

    if (elapsed >= timeout_ms) {
        /* Timeout: go to IDLE and flush RX */
        cc1101_strobe(CC1101_SIDLE);
        cc1101_strobe(CC1101_SFRX);
        return 2; /* timeout */
    }

    /* First byte is packet length */
    pkt_len = cc1101_read_reg(CC1101_RXFIFO);
    if (pkt_len > *length) {
        /* Packet too large for buffer */
        cc1101_strobe(CC1101_SIDLE);
        cc1101_strobe(CC1101_SFRX);
        return 3;
    }

    /* Read payload */
    cc1101_read_burst(CC1101_RXFIFO, data, pkt_len);

    /* Optionally read LQI / RSSI from FIFO if PKTCTRL1 configured so */

    *length = pkt_len;

    /* Go back to IDLE */
    cc1101_strobe(CC1101_SIDLE);

    return 0;
}
