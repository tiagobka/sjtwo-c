#include "FreeRTOS.h"
#include "task.h"

#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "sl_string.h"
#include "uart_printf.h"
#include <stdio.h>
#include <stdlib.h>

#include "cli_handlers.h"
#include "ff.h"
#include "globals.h"
#include "queue.h"
#include <string.h>
void read_file_using_fatfs_pi(int value);
// void write_file_using_fatfs_pi(acceleration__axis_data_s);
sl_string_t fileName = "";
// python nxp-programmer/flash.py --device COM1 -i _build_lpc40xx_freertos/lpc40xx_freertos.bin
// static QueueHandle_t values_queue;
// TaskHandle_t xHandlePlayer;

//-------------------------------------------
//---------------HANDLER-----------------------
//-------------------------------------------
app_cli_status_e cli__player_handler(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                     app_cli__print_string_function cli_output) {

  sl_string_t s = user_input_minus_command_name;
  // uint16_t slenght = sl_string__get_length(s);
  // for (int i = 0; i <= slenght; i++) {
  char c = (char)s[0];
  fileName = s;

  xQueueSend(music_name, &c, 100);
  //}
  // char c = '\n';
  // printf("%c", c);
  // xQueueSend(music_name, &c, 100);

  return APP_CLI_STATUS__SUCCESS;
}
//-------------------------------------------
//---------------HANDLER-----------------------
//-------------------------------------------

void reader(void *p) {
  char x;
  while (1) {
    xQueueReceive(music_name, &x, portMAX_DELAY);
    char c = 'x';
    xQueueSend(data_queue, &c, 100);

    // if (x != '\n')
    // sl_string__append(fileName, x);
  }

  // printf("%c%c%c%c%c%c%c%c", x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]);
}

void player(void *p) {
  char x;
  while (1) {
    xQueueReceive(data_queue, &x, portMAX_DELAY);
    read_file_using_fatfs_pi(0);
  }
}

int main(void) {
  sj2_cli__init();
  // if (!MN_initialized)
  music_name = xQueueCreate(LEN_OF_QUEUE, sizeof(char));
  data_queue = xQueueCreate(1, sizeof(char));

  xTaskCreate(reader, "reader", (512U * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(player, "player", (512U * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  // xTaskCreate(watchdog_task, "Watchdog", (512U * 4) / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  vTaskStartScheduler();
}

/*void write_file_using_fatfs_pi(int value) {
  const char *filename = "tiagofile.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_OPEN_APPEND));

  if (FR_OK == result) {
    char string[64];
    sprintf(string, "TS: %d, x:%i, y: %i, z: %i\n", xTaskGetTickCount(), value.x, value.y, value.z);
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      //  printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);
  } else {
    // printf("ERROR: Failed to open: %s\n", filename);
  }
}*/

void read_file_using_fatfs_pi(int value) {
  // const char *filename = fileName; //todo NOT getting the filename
  const char *filename = "hello.txt"; // had to hardcode it!
  FIL file; // File handle
  UINT bytes_read = 0;
  FRESULT result = f_open(&file, filename, (FA_READ));

  if (FR_OK == result) {
    char string[512];
    if (FR_OK == f_read(&file, string, strlen(string), &bytes_read)) {
    } else {

      printf("ERROR: Failed to read data from file\n");
    }
    for (int i = 0; i < 512; i++) {
      printf("%c", string[i]);
    }
    f_close(&file);
  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}
