#pragma once

#ifndef SerialAT
#define SerialAT Serial1
#endif

#ifndef SerialGPS
#define SerialGPS Serial2
#endif

#define BOARD_MODEM_DTR_PIN                 25
#define BOARD_MODEM_TX_PIN                  26
#define BOARD_MODEM_RX_PIN                  27
#define BOARD_MODEM_PWR_PIN                 4
#define BOARD_MODEM_RI_PIN                  33

// #define BOARD_ADC_PIN                       35
#define BOARD_ADC_CHANNEL                   ADC1_CHANNEL_7

#define BOARD_POWER_ON_PIN                  12 //< LED on board only
#define BOARD_RST_PIN                       5
#define BOARD_IO_BUTTON                     0

#define BOARD_SDCARD_MISO                   2
#define BOARD_SDCARD_MOSI                   15
#define BOARD_SDCARD_SCLK                   14
#define BOARD_SDCARD_CS                     13

#define BOARD_GPS_TX_PIN                    21
#define BOARD_GPS_RX_PIN                    22
#define BOARD_GPS_PPS_PIN                   23
#define BOARD_GPS_WAKEUP_PIN                19

#ifndef TINY_GSM_MODEM_A7670
#define TINY_GSM_MODEM_A7670
#endif

#define BOARD_CHARGER_STATUS_PIN              32

// #define BATTERY_READ_TEST
// #define FORCE_LIGHT_SLEEP