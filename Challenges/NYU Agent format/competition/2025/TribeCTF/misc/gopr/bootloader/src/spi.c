#include "spi.h"

/* ---------- Init: SPI mode 0, 8-bit, modest clock ---------- */
void pl022_init(void) {
    PL022_CR1 = 0;         // Disable SPI while configuring
    PL022_CPSR = 2;        // Clock prescaler (even number)
    PL022_CR0  = (CR0_DSS(8) |
                  CR0_FRF_SPI |
                  CR0_SPO_0 |
                  CR0_SPH_0 |
                  CR0_SCR(0));  // SCR can tune speed
    PL022_CR1 = CR1_SSE;   // Enable SPI, master mode
}

static inline void pl022_wait_not_busy(void) {
    while (PL022_SR & SR_BSY) { }   // Wait for bus idle
}

static inline void pl022_wait_txfifo(void) {
    while (!(PL022_SR & SR_TNF)) { } // Wait until TX FIFO not full
}

static inline uint8_t pl022_read_dr(void) {
    while (!(PL022_SR & SR_RNE)) { } // Wait until RX FIFO not empty
    return (uint8_t)PL022_DR;
}

/* Full-duplex SPI transfer: send header, then read data */
void pl022_transfer_hdr_then_read(const uint8_t *hdr, size_t hdr_len,
                                  uint8_t *rx, size_t len)
{
    size_t txi = 0;
    size_t rxi = 0;
    const size_t total = hdr_len + len;

    while (rxi < total) {
        // Fill TX FIFO
        while ((txi < total) && (PL022_SR & SR_TNF)) {
            uint8_t out = (txi < hdr_len) ? hdr[txi] : 0x00;
            PL022_DR = out;
            txi++;
        }

        // Drain RX FIFO
        while (PL022_SR & SR_RNE) {
            uint8_t in = (uint8_t)PL022_DR;
            if (rxi >= hdr_len) {
                rx[rxi - hdr_len] = in;
            }
            rxi++;
        }
    }

    pl022_wait_not_busy();
}

#if SPI_USE_MANUAL_CS
void spi_cs_init(void) {
    GPIO_DIR |= GPIO_CS_BIT;   // CS pin as output
    GPIO_DATA |= GPIO_CS_BIT;  // CS idle high
}

void spi_cs_assert(void)   { GPIO_DATA &= ~GPIO_CS_BIT; } // CS low
void spi_cs_deassert(void) { GPIO_DATA |=  GPIO_CS_BIT; } // CS high
#endif

/* ---------- Flash helpers ---------- */
void spi_flash_read_id(uint8_t id[3]) {
    const uint8_t cmd = 0x9F;
#if SPI_USE_MANUAL_CS
    spi_cs_assert();
    pl022_transfer_hdr_then_read(&cmd, 1, id, 3);
    spi_cs_deassert();
#else
    pl022_transfer_hdr_then_read(&cmd, 1, id, 3);
#endif
}

void spi_flash_read(uint32_t addr, uint8_t *buf, size_t len) {
    uint8_t hdr[4];
    hdr[0] = 0x03;  // READ command
    hdr[1] = (addr >> 16) & 0xFF;
    hdr[2] = (addr >> 8) & 0xFF;
    hdr[3] = (addr >> 0) & 0xFF;

#if SPI_USE_MANUAL_CS
    spi_cs_assert();
    pl022_transfer_hdr_then_read(hdr, 4, buf, len);
    spi_cs_deassert();
#else
    pl022_transfer_hdr_then_read(hdr, 4, buf, len);
#endif
}
