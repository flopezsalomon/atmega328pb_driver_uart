/*
* atmega328pb_driver_uart.c
*
* Created: 18-05-2024 13:48:43
* Author : Fernando Lopez Salomon
* Description : An simple implementation of USART Driver. As a example of use , was implemented a echo routine.
*/

#ifndef F_CPU
#define F_CPU 16000000UL
#warning "F_CPU not defined! Assuming 16MHz."
#endif

#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "gpio_driver.h"
#include "timer1_driver.h"
#include "usart_driver.h"
#include "task.h"
#include "application.h"

void task_1(void);
void task_2(void);
void task_3(void);

const uint8_t timer_tick_rate = 100;
task_t task[NUMBER_OF_TASK];
port_t status_led;

int main(void)
{
	cli();
	
	gpio_init_pin(&status_led, PORT_STATUS_LED,STATUS_LED, OUTPUT, OFFSET_PORT_LED);
	gpio_config_pin(&status_led);
	gpio_write_pin(&status_led, LOW);
	
	usart_init(&usart[USART_0],USART0_BASE_REG);
	usart_config(&usart[USART_0],BAUDRATE,PARITY_MODE,MODE,DATA_BIT,STOP_BIT);
	
	timer1_driver_set_timerflag(1);
	timer1_driver_init(0,_256_PRESCALER,_100_MS_,0);
	
	task_init(&task[0],200,0,&task_1,UNBLOCKED);
	task_init(&task[1],500,0,&task_2,UNBLOCKED);
	task_init(&task[2],500,0,&task_3,BLOCKED);
	
	sei();
	
	while (1)
	{
		while (1) {
			for (uint8_t i=0; i < NUMBER_OF_TASK; i++) {
				if (task[i].elapsedTime >= task[i].ticks && task[i].state) {
					task[i].fct();
					task[i].elapsedTime = 0;
				}
				task[i].elapsedTime += timer_tick_rate; // Increment the elapsed time of the task by the timer tick rate
			}
			timer1_driver_set_timerflag(0);			// Reset timer flag
			while(!timer1_driver_get_timerflag());	// Wait until the timer flag is set
		}
		return 0;
	}
}

void  task_1(void)
{
	gpio_toogle(&status_led);
}

void task_2(void)
{
	uint8_t input_data_from_user[64];
	uint8_t index = 0;
	
	input_data_from_user[index] = '\0';
	
	while(usart_available(&usart[USART_0])>0){
		input_data_from_user[index] = usart_read(&usart[USART_0]);
		if(input_data_from_user[index] != '\r'){
			index++;
		}
		else
		{
			input_data_from_user[index]   = '\r';
			input_data_from_user[index+1] = '\0';
			index = 0;
			usart_tx_frame(&usart[USART_0],input_data_from_user);
		}
	}
}

void task_3(void)
{
	
}