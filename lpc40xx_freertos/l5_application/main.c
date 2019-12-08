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

char *files[100][32];

uint8_t nFiles;

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

void reader(void *p) {
  FIL file; // File handle
  char filename[32];
  uint64_t fileSizeBytes = 0;
  UINT bytes_read = 0;
  UINT SumBytesRead;
  FILINFO fileInfo;

  while (1) {
    SumBytesRead = 0;

    xQueueReceive(MUSIC_NAME_QUEUE, &filename[0], portMAX_DELAY);
    // printf("reader function received: |%s|", filename);

    FRESULT result = f_open(&file, filename, FA_READ);

    if (FR_OK == result) { // opened file ok
      if (f_stat(filename, &fileInfo) == FR_OK) {
        printf("The File Size is: %d bytes\n", fileInfo.fsize);
        fileSizeBytes = fileInfo.fsize;
      }
      char data[512] = {0};
      while (SumBytesRead < fileSizeBytes) {
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
      printf("ERROR: Failed to open: %s\n", filename);
    }
  }
}

void player(void *p) {
  char data[512] = {0};
  while (1) {
    xQueueReceive(DATA_QUEUE, &data[0], portMAX_DELAY);
    for (int i = 0; i < strlen(data) && i < LEN_OF_DATA; i++) {
      putchar(data[i]);
      // printf("%c, %d", data[i], i);
    }
  }
}

int main(void) {
  // sj2_cli__init();
  rgb_lcd_begin(16, 2, 0);

  VS1053_begin();
  dumpRegs();

  nFiles = 0;

  FATFS fs;
  FRESULT res;
  char buff[256];
  res = f_mount(&fs, "", 1);
  if (res == FR_OK) {
    strcpy(buff, "/");
    res = scan_files(buff);
  }

  // for (int i = 0; i < nFiles; i++) {
  // printf("FileName: %s\n", *files[i]);
  //}

  // FRESULT fr;  /* Return value */
  // DIR dj;      /* Directory search object */
  // FILINFO fno; /* File information */
  // TCHAR pattern = "*.mp3";
  /*
    // f_findfirst(&dj, &fno, path, pattern);
    fr = f_findfirst(&dj, &fno, "", pattern);
    printf("%d\n", fr);
    if (FR_OK == fr) {
      printf("%s\n", fno.altname);
    }*/

  /*for (int i = 0; i < 4; i++) {
    fr = f_findnext(&dj, &fno);
    printf("return is %d", fr);
    printf("%s- %d", fno.altname, fno.fsize);
  }*/

  MUSIC_NAME_QUEUE = xQueueCreate(1, sizeof(TBFILENAME));
  DATA_QUEUE = xQueueCreate(1, LEN_OF_DATA);

  // xTaskCreate(reader, "reader", (512U * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(player, "player", (512U * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
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
        s = fno.fname;
        if (sl_string__contains(s, ".mp3")) {
          *files[nFiles] = fno.fname;
          printf("*files[%d]: %s\n", nFiles, *files[nFiles]);
          nFiles++;
          for (int i = 0; i < sl_string__get_length(s); i++) {
            sendchar(s[i]);
          }
          delay__ms(1000);
          command(0x1);
          command(0x2);

          // files[nFiles][0] = &fno.fname;
          // files[nFiles] = fno.fname;
          // nFiles++;
          // printf("FileName: %s\n", files[nFiles]);
          // printf("%s/%s\n", path, fno.fname);
        }
      }
    }
    f_closedir(&dir);
  }

  return res;
}
