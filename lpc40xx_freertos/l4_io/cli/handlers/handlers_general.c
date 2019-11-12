#include "cli_handlers.h"

#include "FreeRTOS.h"
#include "task.h"

#if (0 != configUSE_TRACE_FACILITY)
static void cli__task_list_print(sl_string_t user_input_minus_command_name, app_cli__print_string_function cli_output);
#endif

app_cli_status_e cli__crash_me(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output) {
  *((uint32_t *)NULL) = 0xDEADBEEF;
  return APP_CLI_STATUS__SUCCESS;
}

#if (0 != configUSE_TRACE_FACILITY)
app_cli_status_e cli__task_list(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                app_cli__print_string_function cli_output) {

  const int sleep_time = sl_string__to_int(user_input_minus_command_name);
  if (sleep_time > 0) {
    vTaskResetRunTimeStats();
    vTaskDelay(sleep_time);
  }

  // re-use user_input_minus_command_name as 'output_string' to save memory:
  sl_string_t output_string = user_input_minus_command_name;
  cli__task_list_print(output_string, cli_output);

  return APP_CLI_STATUS__SUCCESS;
}

static void cli__task_list_print(sl_string_t output_string, app_cli__print_string_function cli_output) {
  // Enum to char : eRunning, eReady, eBlocked, eSuspended, eDeleted
  static const char *const task_status_table[] = {"running", " ready ", "blocked", "suspend", "deleted"};

  // Limit the tasks to avoid heap allocation.
  const unsigned portBASE_TYPE max_tasks = 10;
  TaskStatus_t status[max_tasks];
  uint32_t total_cpu_runtime = 0;
  uint32_t total_tasks_runtime = 0;

  void *unused_cli_param = NULL;
  const uint32_t total_run_time = portGET_RUN_TIME_COUNTER_VALUE();
  const unsigned portBASE_TYPE task_count = uxTaskGetSystemState(&status[0], max_tasks, &total_cpu_runtime);

  sl_string__printf(output_string, "%10s  Status Pr Stack CPU%%          Time\n", "Name");
  cli_output(unused_cli_param, output_string);

  for (unsigned priority_number = 0; priority_number < configMAX_PRIORITIES; priority_number++) {
    /* Print in sorted priority order */
    for (unsigned i = 0; i < task_count; i++) {
      const TaskStatus_t *task = &status[i];
      if (task->uxBasePriority == priority_number) {
        total_tasks_runtime += task->ulRunTimeCounter;

        const unsigned cpu_percent = (0 == total_cpu_runtime) ? 0 : task->ulRunTimeCounter / (total_cpu_runtime / 100);
        const unsigned time_us = task->ulRunTimeCounter;
        const unsigned stack_in_bytes = (sizeof(void *) * task->usStackHighWaterMark);

        sl_string__printf(output_string, "%10s %s %2u %5u %4u %10u us\n", task->pcTaskName,
                          task_status_table[task->eCurrentState], (unsigned)task->uxBasePriority, stack_in_bytes,
                          cpu_percent, time_us);
        cli_output(unused_cli_param, output_string);
      }
    }
  }

  sl_string__printf(output_string, "Overhead: %u uS\n", (unsigned)(total_run_time - total_tasks_runtime));
  cli_output(unused_cli_param, output_string);
}
#else

app_cli_status_e cli__task_list(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                app_cli__print_string_function cli_output) {
  cli_output(argument, "ERROR: configUSE_TRACE_FACILITY needs to be enabled at FreeRTOSConfig.h\n");
  return APP_CLI_STATUS__SUCCESS;
}
#endif /* configUSE_TRACE_FACILITY */

#if (0 != configUSE_TRACE_FACILITY)
app_cli_status_e cli__your_handler(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                   app_cli__print_string_function cli_output) {

  sl_string_t s = user_input_minus_command_name;

  if (sl_string__begins_with_ignore_case(s, "suspend")) {
    // TODO: Use sl_string API to remove the first word, such that variable 's' will equal to 'led0'
    sl_string__trim_start(s, "suspend ");
    // TODO: Or you can do this: char name[16]; sl_string__scanf("%*s %16s", name);

    // Now try to query the tasks with the name 'led0'
    // sl_string__printf(s, "this is it\n");
    TaskHandle_t task_handle = xTaskGetHandle(s);
    if (NULL == task_handle) {
      // note: we cannot use 'sl_string__printf("Failed to find %s", s);' because that would print existing string onto
      // itself
      sl_string__insert_at(s, 0, "Could not find a task with name:");
      cli_output(NULL, s);
    } else {
      // TODO: Use vTaskSuspend()
      sl_string__insert_at(s, 0, "Suspending task with name:");
      vTaskSuspend(task_handle);
    }
  } else if (sl_string__begins_with_ignore_case(s, "resume")) {
    // TODO
    sl_string__trim_start(s, "resume ");
    TaskHandle_t task_handle = xTaskGetHandle(s);
    sl_string__insert_at(s, 0, "Resuming task with name:");
    vTaskResume(task_handle);
  } else {
    cli_output(NULL, "Did you mean to say suspend or resume?\n");
  }

  // sl_string is a powerful string library, and you can utilize the sl_string.h API to parse parameters of a command

  // Sample code to output data back to the CLI
  // Re-use a string to save memory
  // sl_string__printf(s, "Hello back to the CLI\n");
  cli_output(NULL, s);

  return APP_CLI_STATUS__SUCCESS;
}
#endif