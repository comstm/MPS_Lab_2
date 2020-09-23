#include "init.h"

void Intrpt_Init(void);
void Tim_Init(void);

TIM_HandleTypeDef htim7;	// for configuring timer 7
unsigned char tenths = 0;	// tenths of seconds
unsigned char ones = 0;		// whole seconds
unsigned char mins = 0;		// minutes (!)

int main() {
	Sys_Init();
	Intrpt_Init();
	Tim_Init();

	tenths = ones = mins = 0;	// reset all counters
	printf("\033[2J\033[;H");	// erase screen
	fflush(stdout);

	while(1){
	}
}

void Tim_Init(void){
	htim7.Instance = TIM7;			// select timer 7
	htim7.Init.Prescaler = 10799;	// 108MHz/10800 = 10kHz
	htim7.Init.Period = 999;		// 10kHz/1000 = 10Hz => 0.1s
	__HAL_RCC_TIM7_CLK_ENABLE();	// enable the clock

	HAL_TIM_Base_Init(&htim7);		// configure the timer
	HAL_TIM_Base_Start_IT(&htim7);	// start the timer
}

void Intrpt_Init(void){
	HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);	// set interrupt priority
	HAL_NVIC_EnableIRQ(TIM7_IRQn);			// enable timer 7 interrupt
}

void TIM7_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim7);	// clears the flags and all that fun stuff
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM7){	// if timer 7 was the trigger
		tenths++;				// increment tenths
		if(tenths >= 10){		// if tenths >= 10, 1s has passed
			tenths = 0;			// reset tenths
			ones++;				// increment ones
			if(ones >= 60){		// if ones >= 60, 1min has passed
				ones = 0;		// reset ones
				mins++;			// increment mins
			}
		}
		printf("\033[;Htime: %02u:%02u.%u (m:s)\r\n", mins, ones, tenths);	// print the time in mm::ss.(s/10)
	}
}
