#ifndef CC1101_CONFIG_H
#define CC1101_CONFIG_H

#include <stdint.h>
#include "cc1101_regs.h"

typedef struct {
    uint8_t addr;
    uint8_t value;
} cc1101_reg_setting_t;

/* Declare (not define) the arrays */
extern const cc1101_reg_setting_t cc1101_default_settings[];
extern const uint8_t cc1101_default_patable[];
extern const uint8_t cc1101_default_settings_count;

#define CC1101_PATABLE_SIZE (8)

#endif
