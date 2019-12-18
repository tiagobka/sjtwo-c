//
// Created by tiago on 12/5/2019.
//
#include "VS1053.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

void ssp2__init(uint32_t max_clock_khz) {

  if (SPI0TB) {
    lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);
    LPC_SSP0->CR0 = 0b111;
    LPC_SSP0->CR1 = (1 << 1);

  } else {
    LPC_SC->PCONP |= (1 << 20);
    // LPC_SC->PCONP |= (1<<15);
    LPC_SSP2->CR0 = 0b111;
    LPC_SSP2->CR1 = 0b10;
  }
  setSPISpeed(max_clock_khz);
}

void setSPISpeed(uint32_t max_clock_khz) {
  if (SPI0TB) {
    uint8_t divider = 2;
    const uint32_t cpu_clock_khz = clock__get_core_clock_hz() / 1000UL;
    while (max_clock_khz < (cpu_clock_khz / divider) && divider <= 254) {
      divider += 2;
    }
    LPC_SSP0->CPSR = divider;
  } else {
    uint16_t div = (96000 / max_clock_khz);
    if (div % 2) {
      LPC_SSP2->CPSR = (96000 / max_clock_khz);
    } else {
      div++;
      LPC_SSP2->CPSR = (96000 / max_clock_khz);
    }
  }
}

void todo_configure_your_ssp2_pin_functions(void) {

  if (SPI0TB) {
    _miso = gpio__construct_with_function(GPIO__PORT_0, 17, GPIO__FUNCTION_2);
    _mosi = gpio__construct_with_function(GPIO__PORT_0, 18, GPIO__FUNCTION_2);
    _clk = gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCTION_2);
  } else {
    _miso = gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCTION_4);
    _mosi = gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCTION_4);
    _clk = gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCTION_4);
  }

  _cs = gpio__construct_as_output(GPIO__PORT_0, 16);
  gpio__set_as_input(_miso);
  gpio__set_as_output(_clk);
  gpio__set_as_output(_mosi);
}

uint8_t ssp2__exchange_byte_tiago(uint8_t data_out) {

  if (SPI0TB) {
    LPC_SSP0->DR = data_out;
    while (LPC_SSP0->SR & (1 << 4))
      ;
    return (uint8_t)(LPC_SSP0->DR & 0xFF);
  } else {
    LPC_SSP2->DR = data_out;
    while (LPC_SSP2->SR & (1 << 4))
      ;
    return (uint8_t)(LPC_SSP2->DR & 0xFF);
  }
}

uint8_t ssp0__exchange(uint8_t data_out) {
  LPC_SSP0->DR = data_out;
  while (LPC_SSP0->SR & (1 << 4))
    ;
  return (uint8_t)(LPC_SSP0->DR & 0xFF);
}

uint8_t VS1053_begin(void) {

  SPI0TB = true;
  ssp2__init(250);

  playingMusic = false;

  todo_configure_your_ssp2_pin_functions();

  _dcs = gpio__construct_as_output(GPIO__PORT_2, 9);
  _dreq = gpio__construct_as_input(GPIO__PORT_2, 7);
  _reset = gpio__construct_as_output(GPIO__PORT_2, 5);

  gpio__reset(_reset);
  gpio__set(_cs);
  gpio__set(_dcs);

  // configure SPI @125kHz
  // ssp2__initialize(125);
  reset();
  return (sciRead(VS1053_REG_STATUS) >> 4) & 0x0F;
}

void reset(void) {

  gpio__reset(_reset);
  delay__ms(100);
  gpio__set(_reset);

  gpio__set(_cs);
  gpio__set(_dcs);

  sciWrite(VS1053_GPIO_DDR, 0x3);
  sciWrite(VS1053_GPIO_ODATA, 0x0);
  delay__ms(100);
  softReset();
  delay__ms(100);
  sciWrite(VS1053_REG_CLOCKF, 0x6000); // 8BE8
  // sciWrite(VS1053_REG_CLOCKF, 0x21F4);
  setVolume(40, 40);
}

void softReset(void) {
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);
  delay__ms(100);
}

void sciWrite(uint8_t addr, uint16_t data) {
  while (!readyForData())
    ;
  gpio__reset(_cs);

  /*ssp2__exchange_byte_tiago(VS1053_SCI_WRITE);
  ssp2__exchange_byte_tiago(addr);
  ssp2__exchange_byte_tiago(data >> 8);
  ssp2__exchange_byte_tiago(data & 0xFF);*/

  spiwrite(VS1053_SCI_WRITE);
  spiwrite(addr);
  spiwrite(data >> 8);
  spiwrite(data & 0xFF);

  gpio__set(_cs);
}

void spiwrite(uint8_t c) {
  // uint8_t x __attribute__((aligned(32))) = c;
  // spiwrite(&x, 1);
  // printf("Printing 0x%x\n", c);
  ssp2__exchange_byte_tiago(c);
  // ssp0__exchange(c);
}

/*void spiwrite(uint8_t *c, uint16_t num) {
  while (num--) {
    ssp2__exchange_byte_tiago(c[0]);
    c++;
  }
}*/

void setVolume(uint8_t left, uint8_t right) {
  uint16_t v;
  v = left;
  v <<= 8;
  v |= right;

  // todo disable interrupt noInterrupts(); //cli();
  sciWrite(VS1053_REG_VOLUME, v);
  // todo enable interrut interrupts();  //sei();
}

uint16_t decodeTime(void) {
  // todo disable interrupt noInterrupts(); //cli();
  uint16_t ret = sciRead(VS1053_REG_DECODETIME);
  // todo enable interrut interrupts();  //sei();
  return ret;
}

uint16_t sciRead(uint8_t addr) {
  uint16_t data;
  gpio__reset(_cs);

  spiwrite(VS1053_SCI_READ);
  spiwrite(addr);
  delay__us(10);
  data = spiread();
  data <<= 8;
  data |= spiread();

  gpio__set(_cs);
  return data;
}

uint8_t spiread(void) { return ssp2__exchange_byte_tiago(0x00); }

_Bool readyForData(void) { return gpio__get(_dreq); }

void dumpRegs(void) {
  uint16_t temp = sciRead(VS1053_REG_MODE);
  printf("REG_MODE: 0x%x\n", temp);

  temp = sciRead(VS1053_REG_STATUS);
  printf("REG_STATUS: 0x%x\n", temp);

  temp = sciRead(VS1053_REG_CLOCKF);
  printf("REG_CLOCKF: 0x%x\n", temp);
}

void spiwriteData(uint8_t d) {
  // gpio__reset(_dcs);
  ssp2__exchange_byte_tiago(d);
  // ssp0__exchange(d);
  // gpio__set(_dcs);
}

//------------------------ PLAY FILE

_Bool playFullFile(const char *trackname) {}

_Bool startPlayingFile(const char *trackname) {
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
  sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
  sciWrite(VS1053_REG_WRAM, 0);
}
