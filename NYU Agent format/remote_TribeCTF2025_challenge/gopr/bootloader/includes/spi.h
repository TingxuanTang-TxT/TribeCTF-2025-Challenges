#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>
#include <stddef.h>

/* ---------- PL022 (your map: 0x09010000) ---------- */
#define PL022_BASE   0x09010000U
#define PL022_CR0    (*(volatile uint32_t *)(PL022_BASE + 0x00))
#define PL022_CR1    (*(volatile uint32_t *)(PL022_BASE + 0x04))
#define PL022_DR     (*(volatile uint32_t *)(PL022_BASE + 0x08))
#define PL022_SR     (*(volatile uint32_t *)(PL022_BASE + 0x0C))
#define PL022_CPSR   (*(volatile uint32_t *)(PL022_BASE + 0x10))

/* CR0 fields */
#define CR0_DSS(x)   ((x) - 1)          /* 4..16 bits; use 8 -> 7 */
#define CR0_FRF_SPI  (0u << 4)
#define CR0_SPO_0    (0u << 6)          /* CPOL=0 */
#define CR0_SPH_0    (0u << 7)          /* CPHA=0 */
#define CR0_SCR(x)   ((x) << 8)         /* serial clock rate */

/* CR1 bits */
#define CR1_SSE      (1u << 1)          /* enable */
#define CR1_MS       (1u << 2)          /* Master select (0 = master) */
#define CR1_SOD      (1u << 3)

/* SR bits */
#define SR_TFE       (1u << 0)
#define SR_TNF       (1u << 1)
#define SR_RNE       (1u << 2)
#define SR_BSY       (1u << 4)

/* ---------- OPTIONAL: Manual CS via PL061 ---------- */
/* Set to 1 to use GPIO-driven CS; 0 to use PL022 auto-CS */
#define SPI_USE_MANUAL_CS  1

#if SPI_USE_MANUAL_CS
#define PL061_BASE   0x09020000U        /* your PL061 base */
#define GPIO_DATA    (*(volatile uint32_t *)(PL061_BASE + 0x3FC)) /* all bits */
#define GPIO_DIR     (*(volatile uint32_t *)(PL061_BASE + 0x400))
#define GPIO_CS_BIT  (1u << 0)          /* use GPIO0 as CS; adjust if needed */
#endif

/* API */
void     pl022_init(void);

#if SPI_USE_MANUAL_CS
void     spi_cs_init(void);
void     spi_cs_assert(void);
void     spi_cs_deassert(void);
#endif

/* “Header-then-read” helper keeps one transaction:
   sends 'hdr_len' header bytes, then clocks out 'len' read bytes */
void     pl022_transfer_hdr_then_read(const uint8_t *hdr, size_t hdr_len,
                                      uint8_t *rx, size_t len);

/* Flash helpers */
void     spi_flash_read_id(uint8_t id[3]);                     /* RDID 0x9F */
void     spi_flash_read(uint32_t addr, uint8_t *buf, size_t len); /* READ 0x03 */

#endif
