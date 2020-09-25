//----------------------------------
// Lab 2 - Timer Interrupts - Lab02.c
//----------------------------------
// Objective:
//   Build a small game that records user's reaction time.
//

// ---------------------------------
// Imports (NOTE: stdio needed for random number generation)
// ---------------------------------
#include "init.h"
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------
// Function Prototypes
// ---------------------------------
void blinkScreen();
void Init_GPIO();
void Init_Timer();
void Init_Interrupt();

void setupGame();
void randomTime();
void startGame();
void updateScores();
void displayScores();

TIM_HandleTypeDef htim7;	// for configuring timer 7

volatile unsigned int elapsed = 0;  // time, in milliseconds that have passed (reused for the random wait and the game)
unsigned int randomNumber = 0;		// time, in milliseconds that we will wait before surprising the user

// Game mode starts at 0 and returns to 0 with the reset switch is hit
// 0 --> the welcome screen. The game will explain the rules and how it will work and wait for a user input
// 1 --> the game needs to determine the amount of time it is going to wait for
// 2 --> waiting for the timer to trigger the start of the game
// 3 --> the game resets the timer and starts the time for the reaction
// 4 --> waiting for the user to push the button
char gameMode = 0;

// This game saves the last 5 scores
// score1 is the most recent score
// score5 is the score that is about to get bumped
int score1 = -1;
int score2 = -1;
int score3 = -1;
int score4 = -1;
int score5 = -1;
float averageScore = 0;	// average score of all 5 times
unsigned int numScores = 0; // at the beginning of the game there are less than 5 scores so the math gets weird


// ---------------------------------
// Main game and functions
// ---------------------------------

int main() {
	Sys_Init();
	Init_Timer();
	Init_GPIO();
	Init_Interrupt();

	// game is based on a function of modes (described above)
	// note that not all of the possible modes appear here as some wait for pushbutton or timer response to activate
	while (1) {
		if (gameMode == 0){
			setupGame();
		}else if (gameMode == 1){
			randomTime();
		}else if (gameMode == 3){
			startGame();
		}
	}
}

// setupGame clears the screen and all game variables from their previous values. It then displays the game instructions
// and waits for a user response.
void setupGame(){
	printf("\033c\033[36m\033[2J"); // clears all previous displayed items

	// reset game variables
	elapsed = 0;
	randomNumber = 0;
	averageScore = 0;

	// print instructions onto the screen
	printf("\033[1;24HTime to test your reaction time!\r\n"); // instructions (centered)
	printf("\033[3;8HOnce the screen flashes, press pushbutton 0 as fast as possible\r\n");
	printf("\033[5;13HDuring the game, press pushbutton 1 to come back here\r\n");
	printf("\033[12;26HPress any key to continue...\r\n");

	getchar(); // we don't care what is typed, we are just waiting for a response

	gameMode = 1; // once we get our key press, on to the next gameMode
}

// randomTime clears information off of the screen to display a "get ready" instruction and then assigns a random number of milliseconds to
// wait before it will flash the screen and expect a response. Then it starts the timer and switches to the next gameMode.
void randomTime(){
	printf("\033c\033[36m\033[2J"); // clears all previous displayed items
	printf("\033[3;35HGet Ready!\r\n"); // instructions (centered)

	displayScores(); // shows the current scores and average score

	// TODO: the random number generator will always return the same input unless it is seeded. I think people usually do some system time input
	// 		 to do pseudo random based on how many ticks have passed since its been turned on
	randomNumber = rand()%50; // how many tenths of a second should pass until the screen is flashed. Maximum is 5 seconds

	HAL_TIM_Base_Start_IT(&htim7);	// start the timer

	gameMode = 2; // to the next gameMode, which entails waiting for the timer interrupt to trigger
}

// startGame clears the screen, flashes the screen, and rings the bell. It then displays the score, and resets the elapsed time and
// changes the gameMode to wait for a user response from the pushbutton
void startGame(){
	// Clear and redraw display (flash it & sound the bell).
	printf("\033c\033[36m\033[2J"); // clears all previous displayed items

	//printf("\033[30;47m");
	printf("\a\033[s\033[2J\033[u");

	// instructions (centered)
	printf("\033[3;38HGo!\r\n");

	displayScores(); // shows the current scores and average score

	elapsed = 0; // reset the elapsed time. (NOTE: we do not need to start or restart the timer since we are utilizing Timer 7 for both uses)

	gameMode = 4; // the next game mode will wait for a user response from the pushbutton

}

// ---------------------------------
// Utility Functions
// ---------------------------------

// blinkScreen will flash the screen, sound the bell, and clear all information on it
void blinkScreen(){
	printf("\033[30;47m");
	// Clear and redraw display (flash it & sound the bell).
	printf("\a\033[s\033[2J\033[u");
	fflush(stdout);
	HAL_Delay(100);

	printf("\033[37;40m");
	// Clear and redraw display (flash it).
	printf("\033[s\033[2J\033[u");
	fflush(stdout);
}

// updateScores is a helper function that will take the current elapsed time and store it in a socre variable. It will push all of the
// other variables off the list as to only keep 5 at a time.
void updateScores(){
	score5 = score4;
	score4 = score3;
	score3 = score2;
	score2 = score1;
	score1 = elapsed;
	elapsed = 0;
}

// displayScores is another helper function that will display the current scores to the user and calculate and display the average
// of the last 5 scores recorded.
// NOTE: This is an inefficient way to to this when considering lines of code, but the alternative of storing the scores in an array and
// 		 iterating through it is more computationally tasking. Therefore, it was chosen to keep the 5 if statements.
void displayScores(){
	// reset scores just to be sure
	averageScore = 0;
	numScores = 0;
	if (score1 != -1){
		averageScore += score1;
		numScores += 1;
		printf("\033[10;35HTime 1: %d\r\n", score1);
	}else{
		printf("\033[10;35HTime 1: N/A\r\n");
	}
	if (score2 != -1){
		averageScore += score2;
		numScores += 1;
		printf("\033[11;35HTime 2: %d\r\n", score2);
	}else{
		printf("\033[11;35HTime 2: N/A\r\n");
	}
	if (score3 != -1){
		averageScore += score3;
		numScores += 1;
		printf("\033[12;35HTime 3: %d\r\n", score3);
	}else{
		printf("\033[12;35HTime 3: N/A\r\n");
	}
	if (score4 != -1){
		averageScore += score4;
		numScores += 1;
		printf("\033[13;35HTime 4: %d\r\n", score4);
	}else{
		printf("\033[13;35HTime 4: N/A\r\n");
	}
	if (score5 != -1){
		averageScore += score5;
		numScores += 1;
		printf("\033[14;35HTime 5: %d\r\n", score5);
	}else{
		printf("\033[14;35HTime 5: N/A\r\n");
	}
	averageScore = averageScore/numScores;
	unsigned int dispAvgScore = (int)(averageScore); // TODO: I dont understand how to do float printing so this works just as well for right now
	printf("\033[14;35HAverage Time: %u\r\n", dispAvgScore);
	numScores = 0;
}

// ---------------------------------
// Init Functions
// ---------------------------------

void Init_Timer() { // TODO: this should do milliseconds instead of tenths of seconds
	htim7.Instance = TIM7;			// select timer 7
	htim7.Init.Prescaler = 10799;	// 108MHz/10800 = 10kHz
	htim7.Init.Period = 999;		// 10kHz/1000 = 10Hz => 0.1s
	__HAL_RCC_TIM7_CLK_ENABLE();	// enable the clock

	HAL_TIM_Base_Init(&htim7);		// configure the timer
}

void Init_GPIO() {
	__HAL_RCC_GPIOJ_CLK_ENABLE(); 			// enable the clock for GPIO Port J
	GPIO_InitTypeDef J_config;
	J_config.Pin = GPIO_PIN_0;				// select PJ0 (connected to EXTI0, labeled D4 on silkscreen)
	J_config.Mode = GPIO_MODE_IT_RISING;	// set as external interrupt source, rising
	J_config.Pull = GPIO_PULLDOWN;			// enable pull-down resistors
	HAL_GPIO_Init(GPIOJ, &J_config);		// actually configure everything
	asm("nop");
	asm("nop");

	__HAL_RCC_GPIOC_CLK_ENABLE(); 			// enable the clock for GPIO Port J
	GPIO_InitTypeDef C_config;
	C_config.Pin = GPIO_PIN_8;				// select PC8 (connected to EXTI8, labeled D5 on silkscreen)
	C_config.Mode = GPIO_MODE_IT_RISING;	// set as external interrupt source, rising
	C_config.Pull = GPIO_PULLDOWN;			// enable pull-down resistors
	HAL_GPIO_Init(GPIOC, &C_config);		// actually configure everything
	asm("nop");
	asm("nop");
}

void Init_Interrupt(void){
	__HAL_RCC_SYSCFG_CLK_ENABLE();	// enable SYSCFG clock

	HAL_NVIC_EnableIRQ(EXTI0_IRQn); // enable IRQ for EXTI0 (HAL)

	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn); 	// enable IRQ for EXTI8 (HAL)

	HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);	// set interrupt priority
	HAL_NVIC_EnableIRQ(TIM7_IRQn);			// enable timer 7 interrupt
}

// ---------------------------------
// Handlers and Interrupts
// ---------------------------------

void TIM7_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim7);	// clears the flags and all that fun stuff
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM7){	// if timer 7 was the trigger
		if (gameMode == 2){		// if we are in gameMode 2, we are waiting for the random time set to elapse
			elapsed += 1;
			if (elapsed >= randomNumber){ // once the time elapses, switch to gameMode 3
				gameMode = 3;
			}
		}else if (gameMode == 4){	// if we are in gameMode 4, we are waiting for a user pushbutton and keeping track of time passed
			elapsed += 1;
		}
	}
}

void EXTI0_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);	// call the IRQ Handler for EXTI0 (on PJ0)
	asm("nop");
	asm("nop");
}

void EXTI9_5_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);	// call the IRQ Handler for EXTI8 (on PC8)
	asm("nop");
	asm("nop");
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_0){	// if the button connected to PJ0 is pressed
		if (gameMode == 4){ 	// in gameMode 4, we are waiting for the user to press the button so we can record their score
			HAL_TIM_Base_Stop_IT(&htim7); // stop the timer
			asm("nop");
			asm("nop");
			updateScores();
			gameMode = 1;		// go back to assigning a new random time
		}
	}else if (GPIO_Pin == GPIO_PIN_8){ // if the button connected to PC8 is pressed
		gameMode = 0; // reset the game
	}
}
