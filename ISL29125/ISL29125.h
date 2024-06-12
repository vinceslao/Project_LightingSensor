/******************************************************************************************************************************
 *****                                                                                                                    *****
 *****  Name: ISL29125.h                                                                                                  *****
 *****  Date: 06/04/2014                                                                                                  *****
 *****  Auth: Frank Vannieuwkerke                                                                                         *****
 *****  Func: library for Intersil ISL29125 RGB Ambient light sensor with IR blocking filter                              *****
 *****                                                                                                                    *****
 *****  Additional info is available at                                                                                   *****
 *****  http://www.intersil.com/en/products/optoelectronics/ambient-light-sensors/light-to-digital-sensors/ISL29125.html  *****
 *****                                                                                                                    *****
 ******************************************************************************************************************************/

#ifndef ISL29125_H
#define ISL29125_H

#include "mbed.h"

// Common Controls                                                       Used with
//                                                          Operating       IRQ        Status
//                                                          Mode  ADC    assignment    request
#define ISL29125_G      0x01    // Green                        X            X            X
#define ISL29125_R      0x02    // Red                          X            X            X
#define ISL29125_B      0x03    // Blue                         X            X            X
#define ISL29125_RG     0x06    // Red and Green                X            -            -
#define ISL29125_BG     0x07    // Blue and Green               X            -            -
#define ISL29125_RGB    0x05    // Red, Green and Blue          X            -            X
#define ISL29125_STBY   0x04    // Standby                      X            -            -
#define ISL29125_OFF    0x00    // Switch OFF a control         X            X            -
// Unique Controls
#define ISL29125_LTH_W  0x04    // Low interrupt threshold register
#define ISL29125_HTH_W  0x06    // High interrupt threshold register
#define ISL29125_LTH_R  0x02    // Low interrupt threshold register
#define ISL29125_HTH_R  0x03    // High interrupt threshold register
#define ISL29125_375LX  0x00    // Full scale range = 375 lux
#define ISL29125_10KLX  0x08    // Full scale range = 10K lux
#define ISL29125_16BIT  0x00    // ADC resolution = 16 bit
#define ISL29125_12BIT  0x10    // ADC resolution = 12 bit
#define ISL29125_PERS1  0x00    // IRQ when threshold is reached once
#define ISL29125_PERS2  0x04    // IRQ when threshold is reached twice
#define ISL29125_PERS4  0x08    // IRQ when threshold is reached 4 times
#define ISL29125_PERS8  0x0C    // IRQ when threshold is reached 8 times

/** ISL29125 class.
 */

class ISL29125 {
public:
    /**
     *  \brief Create a ISL29125 object connected to I2C bus, irq or sync pin and user-ISR pointer.\n
     *  \param sda       SDA pin.\n
     *  \param scl       SCL pin.\n
     *  \param irqsync   (Optional) Interrupt pin when fptr is also declared OR Sync output when fptr is not declared.\n
     *  \param fptr      (Optional) Pointer to user-ISR (only used with Interrupt pin).\n
     *  \return none\n
     */
    ISL29125(PinName sda, PinName scl, PinName irqsync = NC, void (*fptr)(void) = NULL);

    /**
     *  \brief Read status register.\n
     *         The interrupt status flag is cleared when the status register is read.\n
     *  \param NONE
     *  \return Content of the entire status register.\n
     @verbatim
     bit  Description
     ---  ---------------------------------------------------------
     5,4  RGB conversion     - 00: Inactive
                               01: Green
                               10: Red
                               11: Blue
      2   Brownout status    - 0: No brownout
                               1: Power down or brownout occured
      1   Conversion status  - 0: Conversion is pending or inactive
                               1: Conversion is completed
      0   Interrupt status   - 0: no interrupt occured
                               1: interrupt occured
     @endverbatim
     */
    uint8_t Status(void);

    /**
     *  \brief Read the device identifier.\n
     *  \param none.
     *  \return 0x7D on success.
     */
    uint8_t WhoAmI(void);
    
    /**
     *  \brief Read the channel values (12 or 16-bit - depends on resolution).\n
     *  \param color
     *         ISL29125_R = Red channel.\n
     *         ISL29125_G = Green channel.\n
     *         ISL29125_B = Blue channel.\n
     *         ISL29125_RGB = Red, Green and Blue channels.\n
     *  \param data
     *         Pointer to 16-bit array for storing the channel value(s).\n
     *         Array size: 1 for a single color (Red, Green or Blue) or 3 for all colors.\n
     *  \return bool  1: new data available - 0: no new data available.\n
     */
    bool Read(uint8_t color, uint16_t * data);

    /**
     *  \brief Read/Write the low/high interrupt threshold value.\n
     *         When setIRQonColor is activated, an interrupt will occur when the low or high threshold is exceeded.\n
     *  \param reg
     *         ISL29125_LTH_W = Write 16-bit low threshold.\n
     *         ISL29125_HTH_W = Write 16-bit high threshold.\n
     *         ISL29125_LTH_R = Read 16-bit low threshold.\n
     *         ISL29125_HTH_R = Read 16-bit high threshold.\n
     *  \param thres  16-bit threshold value (only needed when _W parameter is used).\n
     *  \return Written threshold value when called with _W parameter.\n
     *          Stored threshold value when called with _R parameter only.\n
     */
    uint16_t Threshold(uint8_t reg, uint16_t thres=0);

    /**
     *  \brief Read/Write the RGB operating mode value (active ADC channels).\n
     *  \param mode
     *         ISL29125_G = G channel only.\n
     *         ISL29125_R = R channel only.\n
     *         ISL29125_B = B channel only.\n
     *         ISL29125_RG = R and G channel.\n
     *         ISL29125_BG = B and G channel.\n
     *         ISL29125_RGB = R, G and B channel.\n
     *         ISL29125_STBY = Standby (No ADC conversion).\n
     *         ISL29125_OFF = Power down ADC conversion.\n
     *  \return Written value is returned when called with valid parameter, otherwise 0xff is returned.\n
     *          Stored value is returned when called without parameter.\n
     */
    uint8_t RGBmode(uint8_t mode=0xff);

    /**
     *  \brief Read/Write the sensing range parameter.\n
     *  \param range
     *         ISL29125_375LX = Max. value corresponds to 375 lux.\n
     *         ISL29125_10KLX = Max. value corresponds to 10000 lux.\n
     *  \return Written value is returned when called with valid parameter, otherwise 0xff is returned.\n
     *          Stored value is returned when called without parameter.\n
     */
    uint8_t Range(uint8_t range=0xff);

    /**
     *  \brief Read/Write the ADC resolution parameter.\n
     *  \param range
     *         ISL29125_16BIT = 16 bit ADC resolution.\n
     *         ISL29125_12BIT = 12 bit ADC resolution.\n
     *  \return Written value is returned when called with valid parameter, otherwise 0xff is returned.\n
     *          Stored value is returned when called without parameter.\n
     */
    uint8_t Resolution(uint8_t resol=0xff);

    /**
     *  \brief Read/Write the IRQ persistence parameter.\n
     *  \param persist
     *         ISL29125_PERS1 = IRQ occurs when threshold is exceeded once.\n
     *         ISL29125_PERS2 = IRQ occurs when threshold is exceeded twice.\n
     *         ISL29125_PERS4 = IRQ occurs when threshold is exceeded 4 times.\n
     *         ISL29125_PERS8 = IRQ occurs when threshold is exceeded 8 times.\n
     *  \return Written value is returned when called with valid parameter, otherwise 0xff is returned.\n
     *          Stored value is returned when called without parameter.\n
     */
    uint8_t Persist(uint8_t persist=0xff);

    /**
     *  \brief Read/Write the IRQ on conversion done parameter.\n
     *  \param persist true (enabled).\n
     *                 false (disabled).\n
     *  \return Written value is returned when called with valid parameter, otherwise 0xff is returned.\n
     *          Stored value is returned when called without parameter.\n
     */
    uint8_t IRQonCnvDone(uint8_t irqen=0xff);

    /**
     *  \brief Read/Write the IRQ threshold to color assignment parameter.\n
     *  \param RGBmode  ISL29125_OFF = No interrupt.\n
     *                  ISL29125_G = Green interrupt.\n
     *                  ISL29125_R = Red interrupt.\n
     *                  ISL29125_B = Blue interrupt.\n
     *  \return Written value is returned when called with valid parameter, otherwise 0xff is returned.\n
     *          Stored value is returned when called without parameter.\n
     */
    uint8_t IRQonColor(uint8_t RGBmode=0xff);

    /**
     *  \brief Read/Write the active IR compensation parameter.\n
     *  \param ircomp  valid range: between 0..63 or 128..191.\n
     *  \return Written value is returned when called with valid parameter, otherwise 0xff is returned.\n
     *          Stored value is returned when called without parameter.\n
     */
    uint8_t IRcomp(uint8_t ircomp=0xff);

    /**
     *  \brief Start ADC conversion.\n
     *         Only possible when SyncMode is activated.\n
     *  \param None.
     *  \return bool  1: success - 0: fail.
     */
    bool Run(void);

private:
    I2C _i2c;
    FunctionPointer _fptr;
    uint8_t _ismode;             // 0: no irq/sync mode - 1: irq mode - 2: sync mode
    void _alsISR(void);
    void i2cfail(void);
    void readRegs(uint8_t addr, uint8_t * data, uint8_t len);
    uint8_t readReg(uint8_t addr);
    void writeRegs(uint8_t * data, uint8_t len);
};

#endif
