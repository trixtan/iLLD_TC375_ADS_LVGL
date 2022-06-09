/**
 * @file lv_port_disp_templ.h
 *
 */

 /*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_ILI9486L_H
#define LV_PORT_ILI9486L_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES 480
#define MY_DISP_VER_RES 320

/* TFT COMMANDS */
#define TFT_CMD_NOP                                               0x00
#define TFT_CMD_SOFT_RESET                                        0x01
#define TFT_CMD_READ_DISP_ID                                      0x04
#define TFT_CMD_READ_DSI_ERRORS                                   0x05
#define TFT_CMD_READ_STATUS                                       0x09
#define TFT_CMD_READ_POWER_MODE                                   0x0A
#define TFT_CMD_READ_MADCTL                                       0x0B
#define TFT_CMD_READ_PIXEL_FORMAT                                 0x0C
#define TFT_CMD_READ_IMAGE_MODE                                   0x0D
#define TFT_CMD_READ_SIGNAL_MODE                                  0x0E
#define TFT_CMD_READ_SELF_DIAGN_RES                               0x0F
#define TFT_CMD_SLEEP_IN                                          0x10
#define TFT_CMD_SLEEP_OUT                                         0x11
#define TFT_CMD_PARTIAL_MODE_ON                                   0x12
#define TFT_CMD_NORMAL_MODE_ON                                    0x13
#define TFT_CMD_DISPLAY_INVERSION_OFF                             0x20
#define TFT_CMD_DISPLAY_INVERSION_ON                              0x21
#define TFT_CMD_DISPLAY_OFF                                       0x28
#define TFT_CMD_DISPLAY_ON                                        0x29
#define TFT_CMD_COLUMN_ADDRESS_SET                                0x2A
#define TFT_CMD_PAGE_ADDRESS_SET                                  0x2B
#define TFT_CMD_MEMORY_WRITE                                      0x2C
#define TFT_CMD_MEMORY_READ                                       0x2E
#define TFT_CMD_PARTIAL_AREA                                      0x30
#define TFT_CMD_VERTICAL_SCROLL_DEF                               0x33
#define TFT_CMD_TEARING_EFFECT_LINE_OFF                           0x34
#define TFT_CMD_TEARING_EFFECT_LINE_ON                            0x35
#define TFT_CMD_MEMORY_ACCESS_CONTROL                             0x36
#define TFT_CMD_VERT_SCROLL_START_ADDR                            0x37
#define TFT_CMD_IDLE_MODE_OFF                                     0x38
#define TFT_CMD_IDLE_MODE_ON                                      0x39
#define TFT_CMD_INTERFACE_PIXEL_FORMAT                            0x3A
#define TFT_CMD_MEMORY_WRITE_CONT                                 0x3C
#define TFT_CMD_MEMORY_READ_CONT                                  0x3E
#define TFT_CMD_WRITE_TEAR_SCAN_LINE                              0x44
#define TFT_CMD_READ_TEAR_SCAN_LINE                               0x45
#define TFT_CMD_WRITE_BRIGHTNESS_VALUE                            0x51
#define TFT_CMD_READ_BRIGHTNESS_VALUE                             0x52
#define TFT_CMD_WRITE_CTRL_VALUE                                  0x53
#define TFT_CMD_READ_CTRL_VALUE                                   0x54
#define TFT_CMD_WRITE_CONT_ADAPT_BRIGHTNESS_CONTROL_VALUE         0x55
#define TFT_CMD_READ_CONT_ADAPT_BRIGHTNESS_CONTROL_VALUE          0x56
#define TFT_CMD_WRITE_CABC_MINIMUM_BRIGHTNESS                     0x5E
#define TFT_CMD_READ_CABC_MINIMUM_BRIGHTNESS                      0x5F
#define TFT_CMD_READ_FIRST_CHECKSUM                               0xAA
#define TFT_CMD_READ_CONTINUE_CHECKSUM                            0xAF
#define TFT_CMD_READ_ID_1                                         0xDA
#define TFT_CMD_READ_ID_2                                         0xDB
#define TFT_CMD_READ_ID_3                                         0xDC
// Extended commands set
#define TFT_CMD_INTERFACE_MODE_CONTROL                            0xB0
#define TFT_CMD_FRAME_RATE_CONTROL_NORMAL_MODE                    0xB1
#define TFT_CMD_FRAME_RATE_CONTROL_IDLE_MODE                      0xB2
#define TFT_CMD_FRAME_RATE_CONTROL_PARTIAL_MODE                   0xB3
#define TFT_CMD_DISPLAY_INVERSION_CONTROL                         0xB4
#define TFT_CMD_BLANKING_PORCH_CONTROL                            0xB5
#define TFT_CMD_DISPLAY_FUNCTION_CONTROL                          0xB6
#define TFT_CMD_ENTRY_MODE_SET                                    0xB7
#define TFT_CMD_POWER_CONTROL_1                                   0xC0
#define TFT_CMD_POWER_CONTROL_2                                   0xC1
#define TFT_CMD_POWER_CONTROL_3                                   0xC2
#define TFT_CMD_POWER_CONTROL_4                                   0xC3
#define TFT_CMD_POWER_CONTROL_5                                   0xC4
#define TFT_CMD_VCOM_CONTROL_1                                    0xC5
#define TFT_CMD_CABC_CONTROL_2                                    0xC6
#define TFT_CMD_CABC_CONTROL_3                                    0xC8
#define TFT_CMD_CABC_CONTROL_4                                    0xCA
#define TFT_CMD_CABC_CONTROL_5                                    0xCB
#define TFT_CMD_CABC_CONTROL_6                                    0xCC
#define TFT_CMD_CABC_CONTROL_7                                    0xCD
#define TFT_CMD_CABC_CONTROL_8                                    0xCE
#define TFT_CMD_CABC_CONTROL_9                                    0xCF
#define TFT_CMD_NV_MEMORY_WRITE                                   0xD0
#define TFT_CMD_NV_MEMORY_PROTECTION_KEY                          0xD1
#define TFT_CMD_NV_MEMORY_STATUS_READ                             0xD2
#define TFT_CMD_READ_ID4                                          0xD3
#define TFT_CMD_PGAMCTRL                                          0xE0
#define TFT_CMD_NGAMCTRL                                          0xE1
#define TFT_CMD_DIGITAL_GAMMA_CONTROL_1                           0xE2
#define TFT_CMD_DIGITAL_GAMMA_CONTROL_2                           0xE3
#define TFT_CMD_SPI_READING_COMMAND_SETTING                       0xFB


/*********************************************************************************************************************/
/*---------------------------------TFT Display Pin Mapping-----------------------------------------------------------*/
/*********************************************************************************************************************/
/* TFT PIN Mappings */
#define TFT_LCD_BL      &MODULE_P02,6
#define TFT_LCD_RST     &MODULE_P02,6
#define TFT_LCD_DC      &MODULE_P02,4


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_disp_init(void);

void lv_port_disp_reset(void);

void lv_port_disp_switch_off(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_ILI9486L_H*/

#endif /*Disable/Enable content*/
