#include "init.h"

void Intrpt_Init(void);
void Tim_Init(void);

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
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;	// Enable TIM7 clock
	asm("nop");
	asm("nop");

	TIM7->PSC = 43200;	// Set the pre-scaler to slow down ticks
	TIM7->ARR = 250;	// Set the auto reload value
	TIM7->EGR |= TIM_EGR_UG;		// generate update events to auto reload
	TIM7->DIER |= TIM_DIER_UIE;		// enable update interrupts
	TIM7->CR1 |= TIM_CR1_CEN;		// start the timer

}

void Intrpt_Init(void){
	NVIC_SetPriority(TIM7_IRQn, 0);	// set interrupt priority
	NVIC_EnableIRQ(TIM7_IRQn);		// enable timer 7 interrupt
}

void TIM7_IRQHandler(void){
	if ((TIM7->SR & TIM_SR_UIF) == TIM_SR_UIF){	// checks to make sure that the flag for Timer 7 is set
		TIM7->SR &= ~TIM_SR_UIF; // clears the flag

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

	asm("nop");
	asm("nop");
}
