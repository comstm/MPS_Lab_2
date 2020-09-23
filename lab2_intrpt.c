#include "init.h"

void Intrpt_Init(void);
void GPIO_Init(void);

char PB0_count = 0;
char PB1_count = 0;

int main() {
	Sys_Init();
	Intrpt_Init();
	GPIO_Init();

	PB0_count = 0;	// Set the push-button count variables equal to 0 in case they were changed
	PB1_count = 0;

	printf("\033[2J\033[;H");	// erase screen
	fflush(stdout);

	while(1){	// program reactions are all dependent on interrupts, no need for looping instructions
	}
}

void GPIO_Init(void){
	__HAL_RCC_GPIOJ_CLK_ENABLE(); 			// enable the clock for GPIO Port J
	GPIO_InitTypeDef J_config;
	J_config.Pin = GPIO_PIN_0;				// select PJ0 (connected to EXTI0, labeled D4 on silkscreen)
	J_config.Mode = GPIO_MODE_IT_RISING;	// set as external interrupt source, rising
	J_config.Pull = GPIO_PULLDOWN;			// enable pull-down resistors
	HAL_GPIO_Init(GPIOJ, &J_config);		// actually configure everything
	asm("nop");
	asm("nop"); // TODO: do we need these here?

	// PC8 is being used for EXTI8
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;	// enable the clock for GPIO Port C
	GPIOC->MODER &= ~GPIO_MODER_MODER8;		// PC8 input
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR8_0;	// PC8 pull-down (bit 0 = 1)
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR8_1;	// PC8 pull-down (bit 1 = 0)
	EXTI->RTSR	 |= EXTI_RTSR_TR8;			// Set EXTI8 to rising  trigger interrupt source
	EXTI->IMR    |= EXTI_IMR_MR8;			// Un-mask the interrupt TODO: Ask what unmasking means
	SYSCFG->EXTICR[2] = SYSCFG_EXTICR3_EXTI8_PC;	// Routing EXTICR3 to proper port and pin (NOTE: EXTICR[2] is CR3)
	asm("nop");
	asm("nop");

}

void Intrpt_Init(void){
	__HAL_RCC_SYSCFG_CLK_ENABLE();	// enable SYSCFG clock

	HAL_NVIC_EnableIRQ(EXTI0_IRQn); // enable IRQ for EXTI0 (HAL)

	NVIC_EnableIRQ(EXTI9_5_IRQn); 	// enable IRQ for EXTI8 (non-HAL)
}

void EXTI0_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);	// call the IRQ Handler for EXTI0 (on PJ0)
	asm("nop");
	asm("nop");
}

void EXTI9_5_IRQHandler(void){
	if ((EXTI->PR | EXTI_PR_PR8) == EXTI_PR_PR8){ 	// check if pin 8 was the reason this handler was called by checking its flag
		EXTI->PR |= EXTI_PR_PR8;	// clear flag
		PB1_count++;				// increment PB1 press counter
		printf("PB1 Presses: %u\r\n", PB1_count);
	}

	asm("nop"); // Wait period to make sure the flag is cleared
	asm("nop");
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_0){	// if the button connected to PJ0 is pressed
		PB0_count++;				// increment PB0 press counter
		printf("PB0 Presses: %u\r\n", PB0_count);
	}

}
