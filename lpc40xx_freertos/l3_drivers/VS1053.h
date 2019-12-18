//
// Created by tiago on 12/5/2019.
//

#ifndef SJTWO_C_VS1053_H
#define SJTWO_C_VS1053_H

#include "delay.h"
#include "ff.h"
#include "gpio.h"
//#include "ssp2.h"

//-----------Addresses provided by Adafruit-------
#define VS1053_SCI_READ 0x03
#define VS1053_SCI_WRITE 0x02

#define VS1053_REG_MODE 0x00
#define VS1053_REG_STATUS 0x01
#define VS1053_REG_BASS 0x02
#define VS1053_REG_CLOCKF 0x03
#define VS1053_REG_DECODETIME 0x04
#define VS1053_REG_AUDATA 0x05
#define VS1053_REG_WRAM 0x06
#define VS1053_REG_WRAMADDR 0x07
#define VS1053_REG_HDAT0 0x08
#define VS1053_REG_HDAT1 0x09
#define VS1053_REG_VOLUME 0x0B

#define VS1053_GPIO_DDR 0xC017
#define VS1053_GPIO_IDATA 0xC018
#define VS1053_GPIO_ODATA 0xC019

#define VS1053_INT_ENABLE 0xC01A

#define VS1053_MODE_SM_DIFF 0x0001
#define VS1053_MODE_SM_LAYER12 0x0002
#define VS1053_MODE_SM_RESET 0x0004
#define VS1053_MODE_SM_CANCEL 0x0008
#define VS1053_MODE_SM_EARSPKLO 0x0010
#define VS1053_MODE_SM_TESTS 0x0020
#define VS1053_MODE_SM_STREAM 0x0040
#define VS1053_MODE_SM_SDINEW 0x0800
#define VS1053_MODE_SM_ADPCM 0x1000
#define VS1053_MODE_SM_LINE1 0x4000
#define VS1053_MODE_SM_CLKRANGE 0x8000

#define VS1053_SCI_AIADDR 0x0A
#define VS1053_SCI_AICTRL0 0x0C
#define VS1053_SCI_AICTRL1 0x0D
#define VS1053_SCI_AICTRL2 0x0E
#define VS1053_SCI_AICTRL3 0x0F

#define VS1053_DATABUFFERLEN 32
//-----------END- Addresses provided by Adafruit-------

// PINS:
gpio_s _reset, _cs, _dcs, _dreq;
gpio_s _miso, _mosi, _clk;
// END

// VARS:
uint8_t mp3buffer[VS1053_DATABUFFERLEN];
volatile _Bool playingMusic;
_Bool SPI0TB;   // true for SPI0
uint16_t track; // Track Number
uint8_t nFiles; // number of Files in the SD card

// END

uint8_t VS1053_begin(void);                 // done
void reset(void);                           // done
void softReset(void);                       // done
uint16_t sciRead(uint8_t addr);             // done
void sciWrite(uint8_t addr, uint16_t data); // done
// void sineTest(uint8_t n, uint16_t ms);
void spiwrite(uint8_t d); // done
void spiwriteData(uint8_t d);
// void spiwrite(uint8_t *c, uint16_t num); // done
uint8_t spiread(void); // maybe done

uint16_t decodeTime(void);                   // almost done iterrupts missing
void setVolume(uint8_t left, uint8_t right); // almost done todo (dis/e)nable interrupt
void dumpRegs(void);                         // done

void playData(uint8_t *buffer, uint8_t buffsiz);
_Bool readyForData(void); // done
// void applyPatch(const uint16_t *patch, uint16_t patchsize);

/*void GPIO_digitalWrite(uint8_t i, uint8_t val);
void GPIO_digitalWrite(uint8_t i);
uint16_t GPIO_digitalRead(void);
boolean GPIO_digitalRead(uint8_t i);
void GPIO_pinMode(uint8_t i, uint8_t dir);
*/

//--------------------FILE PLAYING

_Bool playFullFile(const char *trackname);
_Bool startPlayingFile(const char *trackname);

#endif // SJTWO_C_VS1053_H
