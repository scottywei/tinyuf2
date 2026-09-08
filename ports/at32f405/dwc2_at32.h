// SPDX-License-Identifier: MIT
// AT32F405 adaptation of TinyUSB's upstream dwc2_at32.h (Ha Thach, 2021).
#pragma once
#include "at32f402_405.h"

#define DWC2_EP_MAX 8
static const dwc2_controller_t _dwc2_controller[] = {
  { .reg_base = OTGFS1_BASE, .irqnum = OTGFS1_IRQn, .ep_count = 4, .ep_fifo_size = 1280 },
  { .reg_base = OTGHS_BASE, .irqnum = OTGHS_IRQn, .ep_count = 8, .ep_fifo_size = 4096 }
};

static inline void dwc2_dcd_int_enable(uint8_t rhport) {
  NVIC_EnableIRQ((IRQn_Type) _dwc2_controller[rhport].irqnum);
}
static inline void dwc2_dcd_int_disable(uint8_t rhport) {
  NVIC_DisableIRQ((IRQn_Type) _dwc2_controller[rhport].irqnum);
}
static inline void dwc2_remote_wakeup_delay(void) {
  for (volatile uint32_t i = 0; i < system_core_clock / 1000; ++i) __NOP();
}
static inline void dwc2_phy_init(dwc2_regs_t* dwc2, uint8_t hs_phy_type) {
  (void) hs_phy_type;
  // Unlike its GHWCFG4 report, AT32F405 supports only 8-bit UTMI (upstream
  // TinyUSB AT32 quirk). Apply before reset and after the generic PHY setup.
  dwc2->gusbcfg &= ~GUSBCFG_PHYIF16;
  // AT32 GCCFG: PHY enable (16), ignore VBUS (21).
  dwc2->stm32_gccfg |= (1u << 16) | (1u << 21);
  dwc2_remote_wakeup_delay();
}
static inline void dwc2_phy_update(dwc2_regs_t* dwc2, uint8_t hs_phy_type) {
  (void) hs_phy_type;
  dwc2->gusbcfg = (dwc2->gusbcfg & ~(GUSBCFG_PHYIF16 | GUSBCFG_TRDT_Msk)) |
                 (9u << GUSBCFG_TRDT_Pos);
  dwc2->stm32_gccfg |= (1u << 16) | (1u << 21);
}
