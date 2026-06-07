/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Tribe “secret” board — Cortex‑R5 + PL011 + PL022 + W25Q80 + PL061
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/units.h"
#include "qemu/error-report.h"
#include "hw/boards.h"
#include "hw/sysbus.h"
#include "hw/arm/boot.h"
#include "hw/char/pl011.h"
#include "hw/ssi/pl022.h"
#include "hw/ssi/ssi.h"
#include "hw/qdev-properties.h"
#include "system/system.h"
#include "qemu/cutils.h"
#include "qom/object.h"
#include "target/arm/cpu-qom.h"

#include "migration/vmstate.h"
#include "hw/char/serial-mm.h"
#include "hw/irq.h"
#include "hw/or-irq.h"
#include "system/block-backend.h"
#include "system/runstate.h"
#include "system/dma.h"
#include "hw/qdev-clock.h"

#define TYPE_TRIBE_MACHINE "tribe-machine"
OBJECT_DECLARE_SIMPLE_TYPE(TribeMachineState, TRIBE_MACHINE)

/* Memory map */
#define ROM_BASE     0x00000000
#define ROM_SIZE     (64 * KiB)
#define SHADOW_BASE  0x00100000
#define SHADOW_SIZE  (64 * KiB)
#define STAGE2_BASE  0x00300000
#define STAGE2_SIZE  (128 * MiB)
#define RAM_BASE     0x08000000
#define RAM_SIZE     (128 * MiB)
#define UART0_BASE   0x09000000
#define SPI0_BASE    0x09010000
#define GPIO0_BASE   0x09020000 // New GPIO controller base address

typedef struct TribeMachineState {
    MachineState parent_obj;
    MemoryRegion rom;
    MemoryRegion rom_shadow;
    MemoryRegion stage2_flash;
    MemoryRegion ram;
    ARMCPU       *cpu;
    SysBusDevice *uart;
    DeviceState  *clk;
    DeviceState  *spi;
    DeviceState  *flash;
    DeviceState  *gpio; // GPIO controller device state
    qemu_irq     uart_irq;
} TribeMachineState;

/* ------------------------------------------------------------ */

static void tribe_board_init(MachineState *machine)
{
    Error *err = NULL;
    TribeMachineState *s = TRIBE_MACHINE(machine);
    qemu_irq cpu_irq;

    /* ---------------- CPU ---------------- */
    s->cpu = ARM_CPU(cpu_create(ARM_CPU_TYPE_NAME("cortex-r5")));
    cpu_irq = qdev_get_gpio_in(DEVICE(s->cpu), 0); // Main IRQ line for the CPU

    /* ---------------- ROM ---------------- */
    memory_region_init_ram(&s->rom, NULL, "tribe.rom", ROM_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), ROM_BASE, &s->rom);

    memory_region_init_rom(&s->rom_shadow, NULL, "tribe.rom.shadow", SHADOW_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), SHADOW_BASE, &s->rom_shadow);

    /* ---------------- STAGE 2 FLASH ---------------- */
    memory_region_init_ram(&s->stage2_flash, NULL, "tribe.stage2_flash", STAGE2_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), STAGE2_BASE, &s->stage2_flash);

    /* ---------------- RAM ---------------- */
    memory_region_init_ram(&s->ram, NULL, "tribe.ram", RAM_SIZE, &err);
    memory_region_add_subregion(get_system_memory(), RAM_BASE, &s->ram);


    /* ---------------- GPIO (PL061) ---------------- */
    s->gpio = DEVICE(sysbus_create_simple("pl061", GPIO0_BASE, cpu_irq));
    // PL061 exposes 8 GPIO pins (0..7), so pins 0,1,2 are safe

    /* ---------------- UART (PL011) ---------------- */
    DeviceState *uartdev = qdev_new(TYPE_PL011);
    object_property_set_str(OBJECT(uartdev), "chardev", "serial0", &err);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(uartdev), &err);
    sysbus_mmio_map(SYS_BUS_DEVICE(uartdev), 0, UART0_BASE);
    // Connect UART IRQ to GPIO0[1]
    sysbus_connect_irq(SYS_BUS_DEVICE(uartdev), 0,
                       qdev_get_gpio_in(s->gpio, 1));
    s->uart = SYS_BUS_DEVICE(uartdev);

    /* ---------------- SPI ctrl (PL022) ---------------- */
    qemu_irq spi_irq = qdev_get_gpio_in(s->gpio, 2); // GPIO0[2] drives SPI IRQ
    s->spi = qdev_new("pl022");
    sysbus_realize_and_unref(SYS_BUS_DEVICE(s->spi), &err);
    sysbus_mmio_map(SYS_BUS_DEVICE(s->spi), 0, SPI0_BASE);
    sysbus_connect_irq(SYS_BUS_DEVICE(s->spi), 0, spi_irq);

    /* Get PL022 child bus */
    BusState *ssi_bus = qdev_get_child_bus(s->spi, "ssi");

    /* ---------------- SPI flash (W25Q80) ---------------- */
    s->flash = qdev_new("m25p80");
    BlockBackend *blk = blk_by_name("flash0");
    if (!blk) {
        error_report("No backend \"flash0\"; run QEMU with -drive file=flash.img,if=none,format=raw,id=flash0");
        exit(EXIT_FAILURE);
    }
    qdev_prop_set_drive(s->flash, "drive", blk);
    qdev_realize_and_unref(s->flash, ssi_bus, &err);

    // Connect flash CS to GPIO0[0]
    qemu_irq flash_cs_in = qdev_get_gpio_in_named(s->flash, "ssi-gpio-cs", 0);
    qdev_connect_gpio_out(s->gpio, 0, flash_cs_in);
}

/* ------------------------------------------------------------ */

static void tribe_board_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc           = "Tribe secret R5 board (PL011 + PL022/W25Q80 + PL061)";
    mc->init           = tribe_board_init;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-r5");
    mc->default_ram_size = RAM_SIZE;
}

static const TypeInfo tribe_board_type_info = {
    .name           = TYPE_TRIBE_MACHINE,
    .parent         = TYPE_MACHINE,
    .instance_size  = sizeof(TribeMachineState),
    .class_init     = tribe_board_class_init,
};

static void tribe_board_register_types(void)
{
    type_register_static(&tribe_board_type_info);
}
type_init(tribe_board_register_types);