#ifndef CC1101_H
#define CC1101_H

#include <stdint.h>
#include "cc1101_regs.h"
#include "cc1101_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * High-level CC1101 driver API.
 */

void cc1101_init(void);
void cc1101_reset(void);
void cc1101_apply_config(void);
void cc1101_write_reg(uint8_t addr, uint8_t value);
uint8_t cc1101_read_reg(uint8_t addr);
void cc1101_write_burst(uint8_t addr, const uint8_t *buffer, uint8_t length);
void cc1101_read_burst(uint8_t addr, uint8_t *buffer, uint8_t length);
uint8_t cc1101_read_status(uint8_t addr);
void cc1101_strobe(uint8_t strobe);

/*
 * Packet helpers (blocking, simple versions).
 * Max length is limited by PKTLEN and FIFO size (64 bytes).
 */
uint8_t cc1101_send_packet(const uint8_t *data, uint8_t length);
uint8_t cc1101_receive_packet(uint8_t *data, uint8_t *length, uint16_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* CC1101_H */
