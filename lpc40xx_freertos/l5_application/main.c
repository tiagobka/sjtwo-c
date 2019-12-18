#include "FreeRTOS.h"
#include "task.h"

#include "GrooveLCD.h"
#include "VS1053.h"
#include "cli_handlers.h"
#include "clock.h"
#include "delay.h"
#include "ff.h"
#include "globals.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "sl_string.h"
#include "uart_printf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// python nxp-programmer/flash.py --device COM4 -i _build_lpc40xx_freertos/lpc40xx_freertos.bin
// static QueueHandle_t values_queue;
// TaskHandle_t xHandlePlayer;
FRESULT scan_files(char *path);

char files[100][32];
/*char *menu[100] = {
        "Play",
        "Pause",
};;*/
// char *globs[100];

gpio_s CS, SO, SCK, SI;
//-------------------------------------------
//---------------HANDLER-----------------------
//-------------------------------------------
app_cli_status_e cli__player_handler(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                     app_cli__print_string_function cli_output) {

  sl_string_t s = user_input_minus_command_name;
  uint16_t slenght = sl_string__get_length(s);

  if (slenght > LEN_OF_NAME) {
    printf("The file name is too big");
    return APP_CLI_STATUS__SUCCESS;
  }
  char c = (char)s[0];

  for (int i = 0; i < LEN_OF_NAME; i++) { // clears variable
    TBFILENAME[i] = (char)0;
  }

  for (int i = 0; i < slenght; i++) {
    TBFILENAME[i] = s[i];
  }
  // TBFILENAME = s;

  xQueueSend(MUSIC_NAME_QUEUE, &TBFILENAME, 100);
  //}
  // char c = '\n';
  // printf("%c", c);
  // xQueueSend(music_name, &c, 100);

  return APP_CLI_STATUS__SUCCESS;
}
//---------------------------------------------
//---------------HANDLER-----------------------
//---------------------------------------------

/*void reader(void *p) {
  FIL file; // File handle
  char filename[32];
  uint8_t index = 0;
  uint64_t fileSizeBytes = 0;
  UINT bytes_read = 0;
  UINT SumBytesRead;
  FILINFO fileInfo;

  // while (!playingMusic);
  while (1) {
    printf("test1");
    xQueueReceive(MUSIC_NAME_QUEUE, &filename[0], portMAX_DELAY);
    printf("test2");

    FRESULT result = f_open(&file, &filename[0], FA_READ);

    SumBytesRead = 0;

    if (FR_OK == result) {
      if (f_stat(filename, &fileInfo) == FR_OK) {
        fileSizeBytes = fileInfo.fsize;
        printf("test3");
      }
      printf("test4");
      char data[32] = {0};

      while (SumBytesRead < fileSizeBytes) {

        if (FR_OK == f_read(&file, data, 1, &bytes_read)) {

          // LOGIC

          if (index > 31) {
            while (!readyForData())
              ;
            index = 0;
          }
          printf("inWhile");
          spiwriteData(data);
          index++;

          SumBytesRead += bytes_read;
        }
      }
    }
  }
}*/

void reader(void *p) {

  FIL file; // File handle
  char filename[32];
  uint64_t fileSizeBytes = 0;
  UINT bytes_read = 0;
  UINT SumBytesRead;
  FILINFO fileInfo;

  vTaskDelay(100);
  while (1) {
    SumBytesRead = 0;

    xQueueReceive(MUSIC_NAME_QUEUE, &filename[0], portMAX_DELAY);
    // printf("reader function received: |%s|", filename);

    FRESULT result = f_open(&file, filename, FA_READ);
    printf("%d", result);
    // FRESULT result = f_open(&file, &filename[0], FA_READ);

    if (FR_OK == result) { // opened file ok
      if (f_stat(filename, &fileInfo) == FR_OK) {
        printf("The File Size is: %d bytes\n", fileInfo.fsize);
        fileSizeBytes = fileInfo.fsize;
      }
      char data[32] = {0};
      while (SumBytesRead < fileSizeBytes) {

        while (!readyForData()) {
          // printf("waiting for data...\n");
        }
        // printf('0x%x 0x%x\n', sciRead(0x08), sciRead(0x09));

        if (FR_OK == f_read(&file, data, LEN_OF_DATA, &bytes_read)) {
          SumBytesRead += bytes_read;
          // printf("bytes read: %d out of %d\n", SumBytesRead, fileSizeBytes);
          xQueueSend(DATA_QUEUE, &data[0], portMAX_DELAY);
        } else {
          printf("failed to read");
        }
      }
      printf("file closed\n");
      f_close(&file);

    } else { // OPEN not OK
             // printf("ERROR: Failed to open: |%s|\n", filename);
      printf("ERROR: Failed to open: |%s|\n", filename);
    }
  }
}

void player(void *p) {
  char data[32] = {0};
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
  sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
  sciWrite(VS1053_REG_WRAM, 0);
  sciWrite(VS1053_REG_DECODETIME, 0x00);
  sciWrite(VS1053_REG_DECODETIME, 0x00);

  // dumpRegs();

  while (1) {
    xQueueReceive(DATA_QUEUE, &data[0], portMAX_DELAY);
    gpio__reset(_dcs);
    // for (int i = 0; i < strlen(data) && i < LEN_OF_DATA; i++) {
    for (int i = 0; i < LEN_OF_DATA; i++) {
      spiwrite(data[i]);
    }
    gpio__set(_dcs);
  }
}

int main(void) {
  // sj2_cli__init();
  rgb_lcd_begin(16, 2, 0);
  VS1053_begin();
  setVolume(0, 0);
  dumpRegs();
  nFiles = 0;
  track = 0;

  FATFS fs;
  FRESULT res;
  char buff[256];
  res = f_mount(&fs, "", 1);
  if (res == FR_OK) {
    strcpy(buff, "/");
    res = scan_files(buff);
  }

  updateFile(track);

  MUSIC_NAME_QUEUE = xQueueCreate(1, sizeof(TBFILENAME));
  DATA_QUEUE = xQueueCreate(1, LEN_OF_DATA);

  // ssp2__init(8000);
  setSPISpeed(8000);
  xQueueSend(MUSIC_NAME_QUEUE, &TBFILENAME, 100);

  xTaskCreate(reader, "reader", 2048U, NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(player, "player", 2048U, NULL, PRIORITY_HIGH, NULL);
  vTaskStartScheduler();
}

FRESULT scan_files(char *path /* Start node to be scanned (***also used as work area***) */) {
  sl_string_t s;
  FRESULT res;
  DIR dir;
  UINT i;
  static FILINFO fno;
  res = f_opendir(&dir, path); /* Open the directory */
  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &fno); /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0)
        break;                    /* Break on error or end of dir */
      if (fno.fattrib & AM_DIR) { /* It is a directory */
        i = strlen(path);
        sprintf(&path[i], "/%s", fno.fname);

        res = scan_files(path); /* Enter the directory */
        if (res != FR_OK)
          break;
        path[i] = 0;
      } else { /* It is a file. */

        char temp[32];
        s = fno.fname;
        if (sl_string__contains(s, ".mp3")) {
          for (int i = 0; i < 32; i++) {
            files[nFiles][i] = fno.fname[i];
          }
          printf("*files[%d]: %s\n", nFiles, files[nFiles]);

          nFiles++;
        }
      }
    }
    f_closedir(&dir);
    printf("There are %d musics in this SD\n", nFiles);
  }
  return res;
}

void updateFile(uint16_t track) {
  for (int i = 0; i < LEN_OF_NAME; i++) { // clears variable
    TBFILENAME[i] = (char)0;
  }
  for (int i = 0; i < LEN_OF_NAME; i++) { // update name
    TBFILENAME[i] = files[track][i];
  }
}
