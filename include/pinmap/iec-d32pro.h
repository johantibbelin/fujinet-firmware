
#ifndef PINMAP_LOLIN_D32_PRO_H
#define PINMAP_LOLIN_D32_PRO_H

#ifdef PINMAP_IEC_D32PRO

/* SD Card */
// pins 12-15 are used to interface with the JTAG debugger
// so leave them alone if we're using JTAG
#ifndef JTAG
#define PIN_CARD_DETECT         GPIO_NUM_12 // fnSystem.h
#define PIN_CARD_DETECT_FIX     GPIO_NUM_15 // fnSystem.h
#endif

#define PIN_SD_HOST_CS          GPIO_NUM_4  // LOLIN D32 Pro
#define PIN_SD_HOST_MISO        GPIO_NUM_19
#define PIN_SD_HOST_MOSI        GPIO_NUM_23
#define PIN_SD_HOST_SCK         GPIO_NUM_18

/* UART */
#define PIN_UART0_RX            GPIO_NUM_3  // fnUART.cpp
#define PIN_UART0_TX            GPIO_NUM_1
#define PIN_UART1_RX            GPIO_NUM_9
#define PIN_UART1_TX            GPIO_NUM_10
#define PIN_UART2_RX            GPIO_NUM_33
#define PIN_UART2_TX            GPIO_NUM_21

/* Buttons */
#define PIN_BUTTON_A            GPIO_NUM_0  // keys.cpp
#define PIN_BUTTON_B            GPIO_NUM_NC
#define PIN_BUTTON_C            GPIO_NUM_14

/* LEDs */
#define PIN_LED_WIFI            GPIO_NUM_2 // led.cpp
#define PIN_LED_BUS             GPIO_NUM_5 // 4 FN

// pins 12-15 are used to interface with the JTAG debugger
// so leave them alone if we're using JTAG
#ifndef JTAG
#define PIN_LED_BT              GPIO_NUM_13
#else
#define PIN_LED_BT              GPIO_NUM_5  // LOLIN D32 PRO
#endif

/* Audio Output */
#define PIN_DAC1                GPIO_NUM_25 // samlib.h

/* Commodore IEC Pins */
#define IEC_HAS_RESET // Reset line is available

#define PIN_IEC_RESET           GPIO_NUM_34
#define PIN_IEC_ATN             GPIO_NUM_32
#define PIN_IEC_CLK_IN          GPIO_NUM_33
#define PIN_IEC_CLK_OUT         GPIO_NUM_33
#define PIN_IEC_DATA_IN         GPIO_NUM_25
#define PIN_IEC_DATA_OUT        GPIO_NUM_25
#define PIN_IEC_SRQ             GPIO_NUM_26

/* Color Computer */
#define PIN_CASS_MOTOR          GPIO_NUM_34 // Second motor pin is tied to +3V
#define PIN_CASS_DATA_IN        GPIO_NUM_33
#define PIN_CASS_DATA_OUT       GPIO_NUM_26

#define PIN_SERIAL_CD           GPIO_NUM_32
#define PIN_SERIAL_RX           GPIO_NUM_9  // fnUartBUS
#define PIN_SERIAL_TX           GPIO_NUM_10

#endif // PINMAP_IEC_D32PRO
#endif // PINMAP_LOLIN_D32_PRO_H