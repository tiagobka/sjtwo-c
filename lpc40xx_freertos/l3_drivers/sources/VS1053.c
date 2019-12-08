//
// Created by tiago on 12/5/2019.
//
#include "VS1053.h"

uint8_t VS1053_begin(void) {
  // hardcoded pins for now
  _cs = gpio__construct_as_output(GPIO__PORT_0, 16);
  _miso = gpio__construct_with_function(GPIO__PORT_0, 17, GPIO__FUNCTION_2);
  _mosi = gpio__construct_with_function(GPIO__PORT_0, 18, GPIO__FUNCTION_2);
  _clk = gpio__construct_with_function(GPIO__PORT_0, 15, GPIO__FUNCTION_2);
  _dcs = gpio__construct_as_output(GPIO__PORT_2, 9);
  _dreq = gpio__construct_as_input(GPIO__PORT_2, 7);
  _reset = gpio__construct_as_output(GPIO__PORT_2, 5);

  gpio__reset(_reset);
  gpio__set(_cs);
  gpio__set(_dcs);

  // configure SPI @125kHz
  ssp2__initialize(125);
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
  sciWrite(VS1053_REG_CLOCKF, 0x6000);
  setVolume(40, 40);
}
void softReset(void) {
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);
  delay__ms(100);
}

void sciWrite(uint8_t addr, uint16_t data) {

  gpio__reset(_cs);

  /*ssp2__exchange_byte(VS1053_SCI_WRITE);
  ssp2__exchange_byte(addr);
  ssp2__exchange_byte(data >> 8);
  ssp2__exchange_byte(data & 0xFF);*/

  spiwrite(VS1053_SCI_WRITE);
  spiwrite(addr);
  spiwrite(data >> 8);
  spiwrite(data & 0xFF);

  gpio__set(_cs);
}

void spiwrite(uint8_t c) {
  // uint8_t x __attribute__((aligned(32))) = c;
  // spiwrite(&x, 1);
  ssp2__exchange_byte(c);
}

/*void spiwrite(uint8_t *c, uint16_t num) {
  while (num--) {
    ssp2__exchange_byte(c[0]);
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

uint8_t spiread(void) { return ssp2__exchange_byte(0x00); }

_Bool readyForData(void) { return gpio__get(_dreq); }

void dumpRegs(void) {
  uint16_t temp = sciRead(VS1053_REG_MODE);
  printf("REG_MODE: 0x%x\n", temp);

  temp = sciRead(VS1053_REG_STATUS);
  printf("REG_STATUS: 0x%x\n", temp);

  temp = sciRead(VS1053_REG_CLOCKF);
  printf("REG_CLOCKF: 0x%x\n", temp);
}
