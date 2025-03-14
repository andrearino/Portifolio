#ifndef __TTGO_AXP192_H__
#define __TTGO_AXP192_H__

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

/**
 *  Função responsável pela configuração do regulador AXP192 utilizando
 *  no kit ttgo v1.4.
 */
void ttgo_power_init(void); 

#ifdef __cplusplus

#include "axp20x.h"
}
#endif


#endif 