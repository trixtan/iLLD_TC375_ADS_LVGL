/*
 * TFTShield.c
 *
 *  Created on: 7 giu 2022
 *      Author: bragante
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_ILI9486L.h"
#include "IfxCpu.h"
#include "IfxPort.h"
#include "Bsp.h"
#include "IfxStm.h"
#include "SPI.h"
#include "STM_Interrupt.h"
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init (void);

static void disp_flush (lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

static void startupTFT(void);
IFX_INLINE void sendCommandToTFT (uint8 command, const uint16 *dataBuffer, Ifx_SizeT count);
static void setWindow(uint32 x1, uint32 y1, uint32 x2, uint32 y2);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_reset(void)
{
    IfxPort_setPinHigh(TFT_LCD_RST);
    waitTime(IfxStm_getTicksFromMilliseconds(&MODULE_STM0, 500));
    IfxPort_setPinLow(TFT_LCD_RST);
    waitTime(IfxStm_getTicksFromMilliseconds(&MODULE_STM0, 500));
    IfxPort_setPinHigh(TFT_LCD_RST);
    waitTime(IfxStm_getTicksFromMilliseconds(&MODULE_STM0, 500));
}

void lv_port_disp_init (void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();
    lv_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */

    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[MY_DISP_HOR_RES * 32]; /*A buffer for 32 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 32); /*Initialize the display buffer*/

    /* Example for 2) */
//    static lv_disp_draw_buf_t draw_buf_dsc_2;
//    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 10];                        /*A buffer for 10 rows*/
//    static lv_color_t buf_2_2[MY_DISP_HOR_RES * 10];                        /*An other buffer for 10 rows*/
//    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 10);   /*Initialize the display buffer*/
    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
    //static lv_disp_draw_buf_t draw_buf_dsc_3;
    //static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*A screen sized buffer*/
    //static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*Another screen sized buffer*/
    //lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MY_DISP_VER_RES * LV_VER_RES_MAX);   /*Initialize the display buffer*/
    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;

    /*Required for Example 3)*/
//    disp_drv.full_refresh = 1;
    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;
    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);

    initSTM();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init (void)
{
    /* disable interrupts */
    boolean interruptState = IfxCpu_disableInterrupts();

    //Initialize QSPI
    initQSPI();

    /* Set the LCD backlight to output push-pull mode */
    IfxPort_setPinModeOutput(TFT_LCD_BL, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Set the LCD reset to output push-pull mode */
    IfxPort_setPinModeOutput(TFT_LCD_RST, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Set the LCD data/command selection to output push-pull mode */
    IfxPort_setPinModeOutput(TFT_LCD_DC, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* enable interrupts again */
    IfxCpu_restoreInterrupts(interruptState);

    startupTFT();

    IfxPort_setPinHigh(TFT_LCD_BL);
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush (lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if ((area->x2 >= area->x1) && area->y2 >= area->y1)
    {
        uint32 pixelsCount = ((area->x2 + 1) - area->x1) * ((area->y2 + 1) - area->y1);
        setWindow(area->x1, area->y1, area->x2, area->y2);
        sendCommandToTFT(TFT_CMD_MEMORY_WRITE, color_p, pixelsCount);
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

//void tft_flush_row_buff( uint32 numberOfPixel, const void * buff)
//{
//    uint16 tx_data;
//
//    if (tft_status == 0)
//    {
//        // calculate the command and address value
//        if (tft_id == 0x9486){
////          tx_data = (0x1 << 8) | 0x2C; // register 0x2C on ILI9486
//            tx_data =  0x2C; // register 0x2C on ILI9486
//        }
//        // wait until Spi is no longer busy (should not busy here)
//        while (IfxQspi_SpiMaster_getStatus(&g_Qspi_Tft.drivers.spiMasterChannel) == SpiIf_Status_busy) {};
//        // send the address to the display
//
//        IfxPort_setPinLow(LCD_DC);  //Command
//        IfxQspi_SpiMaster_exchange(&g_Qspi_Tft.drivers.spiMasterChannel, &tx_data, 0, 1);
//    }
//
//    tft_status = 1; // TFT Busy
//
//    /* wait until Spi is no longer busy (should not busy here) */
//    while (IfxQspi_SpiMaster_getStatus(&g_Qspi_Tft.drivers.spiMasterChannel) == SpiIf_Status_busy) {};
//    /* send the values to the display */
//    IfxPort_setPinHigh(LCD_DC);  //Data
//    IfxQspi_SpiMaster_exchange(&g_Qspi_Tft.drivers.spiMasterChannel, buff, 0, (sint16)(numberOfPixel));
//    tft_terminate_endless_transfer();
//}

void lv_port_disp_switch_off(void)
{
    uint16 uwData[16];

    uwData[0] = 0x0;
    sendCommandToTFT(TFT_CMD_DISPLAY_OFF, uwData, 1); // setting from display supplier
    IfxPort_setPinLow(TFT_LCD_BL);
}

static void startupTFT (void)
{
    uint16 uwData[16];

    uwData[0] = 0x0036;
    uwData[1] = 0x0004;
    uwData[2] = 0x0000;
    uwData[3] = 0x003C;
    uwData[4] = 0x000F;
    uwData[5] = 0x000F;
    uwData[6] = 0x00A4;
    uwData[7] = 0x0002;
    sendCommandToTFT(0xF1, uwData, 8); // setting from display supplier

    uwData[0] = 0x0018;
    uwData[1] = 0x00A3;
    uwData[2] = 0x0012;
    uwData[3] = 0x0002;
    uwData[4] = 0x0032;
    uwData[5] = 0x0012;
    uwData[6] = 0x00FF;
    uwData[7] = 0x0032;
    uwData[8] = 0x0000;
    sendCommandToTFT(0xF2, uwData, 9); // setting from display supplier

    uwData[0] = 0x0040;
    uwData[1] = 0x0000;
    uwData[2] = 0x0008;
    uwData[3] = 0x0091;
    uwData[4] = 0x0004;
    sendCommandToTFT(0xF4, uwData, 5); // setting from display supplier

    uwData[0] = 0x0021;
    uwData[1] = 0x0004;
    sendCommandToTFT(0xF8, uwData, 2); // setting from display supplier

    uwData[0] = 0x19; //VREG1OUT= 5.1250
    uwData[1] = 0x1a; //VREG2OUT= -5.1875
    sendCommandToTFT(TFT_CMD_POWER_CONTROL_1, uwData, 2);

    uwData[0] = 0x45; //VGH,VGL    VGH>=14V.
    uwData[1] = 0x00;
    sendCommandToTFT(TFT_CMD_POWER_CONTROL_2, uwData, 2);

    uwData[0] = 0x33; //Normal mode, increase can change the display quality, while increasing power consumption
    sendCommandToTFT(TFT_CMD_POWER_CONTROL_3, uwData, 1);

    uwData[0] = 0x0068;
    sendCommandToTFT(TFT_CMD_MEMORY_ACCESS_CONTROL, uwData, 1); // Memory Access Control

    uwData[0] = 0x00;
    uwData[1] = 0x28; //VCM_REG[7:0]. <=0X80.
    sendCommandToTFT(TFT_CMD_VCOM_CONTROL_1, uwData, 2);

    //Sets the frame frequency of full color normal mode
    uwData[0] = 0xA0; //0XB0 =70HZ, <=0XB0.0xA0=62HZ
    uwData[1] = 0x11;
    sendCommandToTFT(TFT_CMD_FRAME_RATE_CONTROL_NORMAL_MODE, uwData, 2);

    uwData[0] = 0x02; //2 DOT FRAME MODE,F<=70HZ.
    sendCommandToTFT(TFT_CMD_DISPLAY_INVERSION_CONTROL, uwData, 1);

    uwData[0] = 0x00; //0XB0 =70HZ, <=0XB0.0xA0=62HZ
    uwData[1] = 0x42;
    uwData[2] = 0x3B;
    sendCommandToTFT(TFT_CMD_DISPLAY_FUNCTION_CONTROL, uwData, 3);

    uwData[0] = 0x07;
    sendCommandToTFT(TFT_CMD_ENTRY_MODE_SET, uwData, 1);

    uwData[0] = 0x1F;
    uwData[1] = 0x25;
    uwData[2] = 0x22;
    uwData[3] = 0x0B;
    uwData[4] = 0x06;
    uwData[5] = 0x0A;
    uwData[6] = 0x4E;
    uwData[7] = 0xC6;
    uwData[8] = 0x39;
    uwData[9] = 0x00;
    uwData[10] = 0x00;
    uwData[11] = 0x00;
    uwData[12] = 0x00;
    uwData[13] = 0x00;
    uwData[14] = 0x00;
    sendCommandToTFT(TFT_CMD_PGAMCTRL, uwData, 15);

    uwData[0] = 0x1F;
    uwData[1] = 0x3F;
    uwData[2] = 0x3F;
    uwData[3] = 0x0F;
    uwData[4] = 0x1F;
    uwData[5] = 0x0F;
    uwData[6] = 0x46;
    uwData[7] = 0x49;
    uwData[8] = 0x31;
    uwData[9] = 0x05;
    uwData[10] = 0x09;
    uwData[11] = 0x03;
    uwData[12] = 0x1C;
    uwData[13] = 0x1A;
    uwData[14] = 0x00;
    sendCommandToTFT(TFT_CMD_NGAMCTRL, uwData, 15);

    //Set Interface Pixel Format
    uwData[0] = 0x55;
    sendCommandToTFT(TFT_CMD_INTERFACE_PIXEL_FORMAT, uwData, 1);

    uwData[0] = 0x0000;
    uwData[1] = 0x0000;
    uwData[2] = ((MY_DISP_HOR_RES - 1) & 0xFF00) >> 8;
    uwData[3] = (MY_DISP_HOR_RES - 1) & 0x00FF;
    sendCommandToTFT(TFT_CMD_COLUMN_ADDRESS_SET, uwData, 4);

    uwData[0] = 0x0000;
    uwData[1] = 0x0000;
    uwData[2] = ((MY_DISP_VER_RES - 1) & 0xFF00) >> 8;
    uwData[3] = (MY_DISP_VER_RES - 1) & 0x00FF;
    sendCommandToTFT(TFT_CMD_PAGE_ADDRESS_SET, uwData, 4);

    uwData[0] = 0x0000;
    sendCommandToTFT(TFT_CMD_SLEEP_OUT, uwData, 1);

    //Wait 120 milliseconds
    waitTime(IfxStm_getTicksFromMilliseconds(&MODULE_STM0, 120));

    uwData[0] = 0x0000;
    sendCommandToTFT(TFT_CMD_DISPLAY_ON, uwData, 1);
}

IFX_INLINE void sendCommandToTFT (uint8 command, const uint16 *dataBuffer, Ifx_SizeT count)
{
    uint16 cmd;

    cmd = command;
    IfxPort_setPinLow(TFT_LCD_DC); //Signal command will be transmitted
    transferDataToTFT(&cmd, 1);
    IfxPort_setPinHigh(TFT_LCD_DC); //Signal data will be transmitted
    transferDataToTFT(dataBuffer, count);
}

static void setWindow(uint32 x1, uint32 y1, uint32 x2, uint32 y2)
{
    uint16 uwData[5];

    uwData[0] = (uint16) (x1 >> 8);
    uwData[1] = (uint16) x1 & 0xFF;
    uwData[2] = (uint16) ((x2 - 1) >> 8);
    uwData[3] = (uint16) (x2 - 1) & 0xFF;
    sendCommandToTFT(TFT_CMD_COLUMN_ADDRESS_SET, uwData, 4);

    uwData[0] = (uint16) (y1 >> 8);
    uwData[1] = (uint16) y1 & 0xFF;
    uwData[2] = (uint16) ((y2 - 1) >> 8);
    uwData[3] = (uint16) (y2 - 1) & 0xFF;
    sendCommandToTFT(TFT_CMD_PAGE_ADDRESS_SET, uwData, 4);

    uwData[0] = 0x00;
    sendCommandToTFT(TFT_CMD_MEMORY_WRITE, uwData, 1);
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}
#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif

