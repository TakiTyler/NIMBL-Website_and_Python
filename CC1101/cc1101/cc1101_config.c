#include "cc1101_config.h"

/*
 * CC1101 Register Configuration
 * -----------------------------
 * This table defines a complete RF configuration for ~433 MHz operation,
 * using GFSK modulation at approximately 4.8 kbps with ~100 kHz channel
 * bandwidth. This profile is derived from TI SmartRF Studio baseline
 * settings and is known to work reliably on CC1101 and most CC1101-compatible
 * clone modules.
 *
 * All registers are written using the sequence defined in cc1101_apply_config().
 * PATABLE is written separately.
 */

const cc1101_reg_setting_t cc1101_default_settings[] = {

    /* ==== GPIO / INTERFACE ==== */
    { CC1101_IOCFG2,   0x06 },  // GDO2: Asserted when sync word is sent/received
    { CC1101_IOCFG0,   0x07 },  // GDO0: High when packet CRC OK (PKTSTATUS.CRC_OK)

    /* ==== FIFO / SYNC WORD ==== */
    { CC1101_FIFOTHR,  0x47 },  // High FIFO threshold for TX/RX
    { CC1101_SYNC1,    0xD3 },  // Sync word high byte
    { CC1101_SYNC0,    0x91 },  // Sync word low byte

    /* ==== PACKET ENGINE ==== */
    { CC1101_PKTLEN,   0x3D },  // Fixed packet length = 61 bytes
    { CC1101_PKTCTRL1, 0x04 },  // CRC autoflush enabled, no address filtering
    { CC1101_PKTCTRL0, 0x05 },  // Whitening enabled, CRC enabled, fixed length
    { CC1101_ADDR,     0x00 },  // Device address (not used here)
    { CC1101_CHANNR,   0x00 },  // Logical RF channel number

    /* ==== FREQUENCY SYNTHESIZER ==== */
    { CC1101_FSCTRL1,  0x08 },  // Frequency IF setting; typical = 8
    { CC1101_FSCTRL0,  0x00 },

    /* ==== RF CARRIER FREQUENCY (433 MHz) ==== */
    /*
     * Frequency = (FREQ[23:0] / 2^16) * 26 MHz
     * FREQ = 0x10A762 ≈ 433.0 MHz
     */
    { CC1101_FREQ2,    0x10 },
    { CC1101_FREQ1,    0xA7 },
    { CC1101_FREQ0,    0x62 },

    /* ==== MODEM CONFIGURATION ==== */
    /*
     * MDMCFG4/3: Data rate and channel bandwidth.
     * MDMCFG2: Modulation format, sync behavior, DC filter.
     * MDMCFG1/0: FEC, preamble/sync behavior.
     */
    { CC1101_MDMCFG4,  0xCA },  // RX filter bandwidth ~101 kHz
    { CC1101_MDMCFG3,  0x83 },  // Data rate ≈ 4.8 kbps
    { CC1101_MDMCFG2,  0x93 },  // GFSK, 16/16 sync, DC filter enabled
    { CC1101_MDMCFG1,  0x22 },  // FEC disabled, 4-byte preamble
    { CC1101_MDMCFG0,  0xF8 },  // Channel spacing: 199.95 kHz

    /* ==== MODULATION DEVIATION ==== */
    { CC1101_DEVIATN,  0x34 },  // Deviation ≈ 20 kHz

    /* ==== RADIO STATE MACHINE ==== */
    { CC1101_MCSM1,    0x30 },  // After TX/RX, stay in RX
    { CC1101_MCSM0,    0x18 },  // Auto-calibrate when going from IDLE → RX/TX

    /* ==== AUTOMATIC GAIN CONTROL === */
    { CC1101_FOCCFG,   0x16 },  // Frequency offset compensation
    { CC1101_BSCFG,    0x6C },  // Bit synchronization config
    { CC1101_AGCCTRL2, 0x43 },  // AGC target + gain table settings
    { CC1101_AGCCTRL1, 0x40 },  // AGC reference level
    { CC1101_AGCCTRL0, 0x91 },  // AGC enable + hysteresis

    /* ==== FRONT END + POWER ==== */
    { CC1101_FREND1,   0x56 },  // Front-end TX: LNA/PT settings
    { CC1101_FREND0,   0x10 },  // Use PA_TABLE[0] for power level

    /* ==== FREQUENCY SYNTH CALIBRATION ==== */
    { CC1101_FSCAL3,   0xE9 },
    { CC1101_FSCAL2,   0x2A },
    { CC1101_FSCAL1,   0x00 },
    { CC1101_FSCAL0,   0x1F },

    /* ==== TEST SETTINGS (TI RECOMMENDED) ==== */
    { CC1101_TEST2,    0x81 },
    { CC1101_TEST1,    0x35 },
    { CC1101_TEST0,    0x09 },
};

/*
 * Power Amplifier Table
 * ---------------------
 * For CC1101, PA_TABLE[0] holds the active output power level when FREND0.PATABLE
 * selects index 0. Value 0xC0 corresponds to roughly +10 dBm at 433 MHz on
 * most common CC1101 module PCBs.
 */
const uint8_t cc1101_default_patable[] = {
    0xC0,   // PA_TABLE[0]: +10 dBm nominal
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t cc1101_default_settings_count =
    sizeof(cc1101_default_settings) / sizeof(cc1101_default_settings[0]);