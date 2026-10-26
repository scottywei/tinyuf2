// SPDX-License-Identifier: MIT
#ifndef W25Q32_H_
#define W25Q32_H_

#include <stdbool.h>
#include <stdint.h>

void w25q32_init(void);
bool w25q32_read(uint32_t offset, void* data, uint32_t len);
bool w25q32_write(uint32_t offset, void const* data, uint32_t len);

#endif
