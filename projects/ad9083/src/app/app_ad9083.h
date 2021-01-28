#ifndef __APP_AD9083_H
#define __APP_AD9083_H

#include <stdint.h>
#include <stdbool.h>

int32_t app_ad9083_init(uint8_t uc);

bool app_ad9083_check_sysref_rate(uint32_t lmfc, uint32_t sysref);

#endif /* __APP_AD9083_H */
