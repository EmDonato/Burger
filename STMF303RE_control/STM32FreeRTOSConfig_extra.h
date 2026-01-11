// STM32FreeRTOSConfig_extra.h

#pragma once

// Abilito i QueueSet (di default sono disabilitati)
#define configUSE_QUEUE_SETS 1
#define configTICK_RATE_HZ    ((TickType_t)100)