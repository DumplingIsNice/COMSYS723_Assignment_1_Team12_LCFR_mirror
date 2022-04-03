/*
 * service_VGA.c
 *
 *  Created on: 30 Mar 2022
 *      Author: Hao Lin
 */

#include "service_VGA.h"

void service_VGA()
{
	alt_up_char_buffer_dev *char_buf;
	alt_up_pixel_buffer_dma_dev *pixel_buf;

	// No need for an init function, just init in the task!
	pixel_buf = alt_up_pixel_buffer_dma_open_dev(VIDEO_PIXEL_BUFFER_DMA_NAME);
	if(pixel_buf == NULL){
		printf("can't find pixel buffer device\n");
	}

	// Reset the display
	alt_up_pixel_buffer_dma_clear_screen(pixel_buf, 0);

	char_buf = alt_up_char_buffer_open_dev("/dev/video_character_buffer_with_dma");
	if(char_buf == NULL){
		printf("can't find char buffer device\n");
	}

	alt_up_char_buffer_clear(char_buf);
	printf("VGA Controllers initialised!\n");

	printf("service_VGA Running\n");

	char VGA_print_buffer[] = "";

	// Local Buffers to hold value for display
	uint response_time_vals[RESPONSE_TIME_BUF_SIZE] = {0};

	double freq_vals[FREQ_DATA_BUF_SIZE] = {0};
	double roc_vals[ROC_DATA_BUF_SIZE] = {0};
	uint n = 99; // Iterator
	uint* n_p = &n;

	double threshold_freq = 12.2;
	double threshold_roc = 12.3;

	char sys_status[] = "Stable";

	static uint service_VGA_counter = 0;

	uint max = 1;
	uint min = 2;
	uint avg = 3;
	uint sys = 5;

	Line line_freq, line_ROC;

	while(TRUE)
	{
		// Multipurpose iterators
		uint i = 0;
		uint j = 0;

		/* Data Reading */

		// Empties Freq data queue
		if (uxQueueMessagesWaiting(Q_freq_calc_values) != 0)
		{
			//printf("!!!!!!!!!!!!Waiting freq_queue_sem!!!!!!!!!!!!\n");
			if (xSemaphoreTake(freq_queue_sem, portMAX_DELAY) == pdTRUE)
			{
				//printf("!!!!!!!!!!!!Emptying FREQ Queue!!!!!!!!!!!!\n");
				empty_queue(EMPTY_FREQ, freq_vals, n_p);
				xSemaphoreGive(freq_queue_sem);
				//printf("!!!!!!!!!!!!Finished Emptying freq_queue_sem!!!!!!!!!!!!\n");
				vTaskDelay(15);
			} else {
				printf("freq_queue_sem Semaphore cannot be taken!\n");
			}
		}

		// Empties RoC data queue

		/* Draws Graph */

		// Clear old graph
		alt_up_pixel_buffer_dma_draw_box(pixel_buf, 101, 210, 600, 360, 0, 0);
//		alt_up_pixel_buffer_dma_draw_box(pixel_buf, 101, 380, 600, 460, 0, 0);

		// Freq
		alt_up_pixel_buffer_dma_draw_hline(pixel_buf, 100, 590, 360, ((0x3ff << 20) + (0x3ff << 10) + (0x3ff)), 0);
		alt_up_pixel_buffer_dma_draw_vline(pixel_buf, 100, 210, 360, ((0x3ff << 20) + (0x3ff << 10) + (0x3ff)), 0);

		// ROC
		alt_up_pixel_buffer_dma_draw_hline(pixel_buf, 100, 590, 460, ((0x3ff << 20) + (0x3ff << 10) + (0x3ff)), 0);
		alt_up_pixel_buffer_dma_draw_vline(pixel_buf, 100, 380, 460, ((0x3ff << 20) + (0x3ff << 10) + (0x3ff)), 0);

		alt_up_char_buffer_string(char_buf, "Freq(Hz)", 0, 26);
		i = 22;
		alt_up_char_buffer_string(char_buf, "52", 10, i+=5);
		alt_up_char_buffer_string(char_buf, "50", 10, i+=5);
		alt_up_char_buffer_string(char_buf, "48", 10, i+=5);
		alt_up_char_buffer_string(char_buf, "46", 10, i+=5);

		alt_up_char_buffer_string(char_buf, "df/dt", 0, 46);
		alt_up_char_buffer_string(char_buf, "(Hz/s)", 0, 47);
		alt_up_char_buffer_string(char_buf, "(Abs)", 0, 48);

		for(uint k=0;k<99;++k){ //i here points to the oldest data, j loops through all the data to be drawn on VGA
			if (((int)(freq_vals[(n+k)%100]) > MIN_FREQ) && ((int)(freq_vals[(n+k+1)%100]) > MIN_FREQ)){
				//Calculate coordinates of the two data points to draw a line in between
				//Frequency plot
				line_freq.x1 = FREQPLT_ORI_X + FREQPLT_GRID_SIZE_X * k;
				line_freq.y1 = (int)(FREQPLT_ORI_Y - FREQPLT_FREQ_RES * (freq_vals[(n+k)%100] - MIN_FREQ));

				line_freq.x2 = FREQPLT_ORI_X + FREQPLT_GRID_SIZE_X * (k + 1);
				line_freq.y2 = (int)(FREQPLT_ORI_Y - FREQPLT_FREQ_RES * (freq_vals[(n+k+1)%100] - MIN_FREQ));

				//Frequency RoC plot
//				line_ROC.x1 = ROCPLT_ORI_X + ROCPLT_GRID_SIZE_X * k;
//				line_ROC.y1 = (int)(ROCPLT_ORI_Y - ROCPLT_ROC_RES * dfreq[(i+j)%100]);
//
//				line_ROC.x2 = ROCPLT_ORI_X + ROCPLT_GRID_SIZE_X * (k + 1);
//				line_ROC.y2 = (int)(ROCPLT_ORI_Y - ROCPLT_ROC_RES * dfreq[(i+j+1)%100]);

				//Draw
				alt_up_pixel_buffer_dma_draw_line(pixel_buf, line_freq.x1, line_freq.y1, line_freq.x2, line_freq.y2, 0x3ff << 0, 0);
				alt_up_pixel_buffer_dma_draw_line(pixel_buf, line_ROC.x1, line_ROC.y1, line_ROC.x2, line_ROC.y2, 0x3ff << 0, 0);
			}
		}

		/* Display Information */

		alt_up_char_buffer_string(char_buf, "Team 12", 0, 1);
		alt_up_char_buffer_string(char_buf, "LCFR", 10, 1);

		i = 2;
		j = 0;

		sprintf(VGA_print_buffer, "service_VAG Running Counter: %d", service_VGA_counter++);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 0, i+=2);

		alt_up_char_buffer_string(char_buf, "By:", 0, i+=2);
		alt_up_char_buffer_string(char_buf, "Hao Lin", 1, i+=2);
		alt_up_char_buffer_string(char_buf, "Chamith Nanayakkara", 1, i+=2);

		i += 2;

		alt_up_char_buffer_string(char_buf, "System Infomation: ", 0, i+=2);
		sprintf(VGA_print_buffer, "Status: %s", sys_status);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 0, i+=2);

		j = i;
		i = 0;

		alt_up_char_buffer_string(char_buf, "Thresholds:", 40, i+=2);
		sprintf(VGA_print_buffer, "Freq: %.2f", threshold_freq);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 42, i+=2);
		sprintf(VGA_print_buffer, " RoC: %.2f", threshold_roc);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 42, i+=2);

		alt_up_char_buffer_string(char_buf, "Threshold Exceed Response Time:", 40, i+=2);
		sprintf(VGA_print_buffer, "Max: %d", max);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 42, i+=2);
		sprintf(VGA_print_buffer, "Min: %d", min);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 42, i+=2);
		sprintf(VGA_print_buffer, "Avg: %d", avg);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 42, i+=2);
		sprintf(VGA_print_buffer, "Total System Up-time: %d", sys);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 40, i+=2);

		i = j;
		i += 2;

		sprintf(VGA_print_buffer, "Five most recent freq values: %.2f, %.2f, %.2f, %.2f, %.2f",
								 freq_vals[n], freq_vals[n+1], freq_vals[n+2], freq_vals[n+3], freq_vals[n+4]);
			alt_up_char_buffer_string(char_buf, (char*)VGA_print_buffer, 0, i+=2);

			alt_up_char_buffer_string(char_buf, " Five most recent ROC values:", 0, i+=2);

#ifdef MOCK_RESPONSE
		if (uxQueueMessagesWaiting(Q_response_time) != 0)
		{
			printf("!!!!!!!!!!!!Waiting!!!!!!!!!!!!\n");
			if (xSemaphoreTake(response_time_sem, portMAX_DELAY) == pdTRUE)
			{
				printf("!!!!!!!!!!!!Emptying Queue!!!!!!!!!!!!\n");
				empty_response_queue(response_time_vals);
				xSemaphoreGive(response_time_sem);
				printf("!!!!!!!!!!!!Finished Emptying Queue!!!!!!!!!!!!\n");
				memset(&response_time_vals, 0, RESPONSE_TIME_BUF_SIZE);
				vTaskDelay(15);
			} else {
				printf("response_time_sem Semaphore cannot be taken!\n");
			}
		}
#endif
	}
}

void empty_queue(char mux, double* local_vals, uint* n)
{
	double data = 0;
	uint i = 0;
	QueueHandle_t Q;

	switch (mux)
		{
		case EMPTY_FREQ:
			Q = Q_freq_calc_values;
			break;
		case EMPTY_ROC:
			Q = Q_roc_calc_values;
			break;
		default:
			return;
		}

	while(uxQueueMessagesWaiting(Q) != 0)
	{
		xQueueReceive(Q, &data, portMAX_DELAY);
		printf("Emptied values %d is %f\n", i, data);
		(*n) = ++(*n)%100;
		local_vals[(*n)] = data;
		printf("Local values %d is %f\n", (*n), local_vals[(*n)]);
		i++;
	}
}

void empty_response_queue(uint* local_vals)
{
	uint data = 0;
	uint i = 0;

	while(uxQueueMessagesWaiting(Q_response_time) != 0)
	{
		xQueueReceive(Q_response_time, &data, portMAX_DELAY);
		printf("Emptied values %d is %d\n", i, data);
//		local_vals[i] = data;
//		printf("Emptied values %d is %d\n", i, local_vals[i]);
		i++;
	}
}

