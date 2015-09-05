#ifndef _HAL_DRIVER_H_
#define _HAL_DRIVER_H_


typedef struct
{
    TActive act;

    uint8_t wfi_enable;
} HalDriver;

extern HalDriver g_HalDriver;
extern TEvt queue_HalDriver[50];

void HalDriver_ctor(HalDriver *me);

void RTC_Configuration(void);

#endif
