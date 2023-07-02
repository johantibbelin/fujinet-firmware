/* FujiNet Hardware Pin Mapping */
#ifdef PINMAP_ADAMV1

/* SD Card */
#define PIN_CARD_DETECT         GPIO_NUM_12 // fnSystem.h
#define PIN_CARD_DETECT_FIX     GPIO_NUM_15 // fnSystem.h
#define PIN_SD_HOST_CS          GPIO_NUM_5 //fnFsSD.cpp
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
#define PIN_BUTTON_B            GPIO_NUM_34
#define PIN_BUTTON_C            GPIO_NUM_14

/* LEDs */
#define PIN_LED_WIFI            GPIO_NUM_2 // led.cpp
#define PIN_LED_BUS             GPIO_NUM_4

// pins 12-15 are used to interface with the JTAG debugger
// so leave them alone if we're using JTAG
#if !defined(JTAG)
#define PIN_LED_BT              GPIO_NUM_13
#else
#define PIN_LED_BT              GPIO_NUM_4
#endif

/* Atari SIO Pins */
#define PIN_INT                 GPIO_NUM_26 // sio.h
#define PIN_PROC                GPIO_NUM_22
#define PIN_CKO                 GPIO_NUM_32
#define PIN_CKI                 GPIO_NUM_27
#define PIN_MTR                 GPIO_NUM_36
#define PIN_CMD                 GPIO_NUM_39

/* Audio Output */
#define PIN_DAC1                GPIO_NUM_25 // samlib.h

/* Pins for AdamNet */
#define PIN_ADAMNET_RESET       GPIO_NUM_26

/* Pins for Adam USB */
#define PIN_USB_DP              GPIO_NUM_27      // D+
#define PIN_USB_DM              GPIO_NUM_32      // D-

#endif /* PINMAP_ADAMV1 */
