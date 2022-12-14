/*
 * tasks.h
 *
 *  Created on: 25 Mar 2022
 *      Author: Hao Lin
 */

#ifndef RTOS_TASKS_H_
#define RTOS_TASKS_H_

#include "..\customAPI\project.h"

// Definition of Task Stacks
#define TASK_STACKSIZE       2048

// Definition of Task Priorities
#define CALC_FREQ_ROC_PRIORITY 		configMAX_PRIORITIES - 1
#define HANDLE_LOAD_PRIORITY 		configMAX_PRIORITIES - 2
#define HANDLE_KEYBOARD_PRIORITY 	configMAX_PRIORITIES - 3
#define SERVICE_VGA_PRIORITY 		configMAX_PRIORITIES - 4

// Definition of Periods
#define RESPONSE_TIME_WINDOW 	200
#define VALIDATION_TIME_WINDOW  500

// Global Variable Semaphores
SemaphoreHandle_t global_system_status_sem;

// Task handles used to delete a task
TaskHandle_t xHandle;

/* Task Function Declarations */

// Creates message queue, semaphore and other inter-task data packages
int init_OS_data_structs();

// Creates the tasks in the system
int init_create_tasks();

#endif /* RTOS_TASKS_H_ */
