/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif
#include <cr_section_macros.h>
#include <cstdio>
#include <iostream>

#define TICKRATE_HZ1 (100)	/* 100 ticks per second */

bool flag;
volatile int h = 0;
volatile int m = 0;
volatile int s = 0;
volatile int pulssi = 0;
static int q = 0;
static int ms = 0;

extern "C" {

class Timer {
public:
	Timer(int min = 1, int sec = 0, int ms =0, bool recurring = false);
	virtual ~Timer();
	bool IsRecurring(); // is this a recurring timer?
	bool Tick(); // make timer tick once, returns true if timer expired,
private:
	int tickit;
	int tickeja;
};

bool Timer::Tick() {
	// Tikataan kerran

	if (tickit != tickeja) {
		tickeja++;
		return false;
	}

	if ( tickit <= tickeja) {
		tickeja = 0;
		return true;
	}

}

bool Timer::IsRecurring() {
}

// returns the interrupt enable state before entering critical section
bool enter_critical(void)
{
	uint32_t pm = __get_PRIMASK();
	__disable_irq();
	return (pm & 1) == 0;
}
// restore interrupt enable state
void leave_critical(bool enable)
{
	if(enable) __enable_irq();
}

Timer::Timer(int min, int sec, int ms, bool recurring) {
	tickit = (sec * 100) + (min * 60 * 100) + (ms / 10);
	tickeja = 0;
}

Timer::~Timer() {

}


void SysTick_Handler(void)
{
	// implement real time clock with hours, minutes and seconds here


	ms++;
	flag = true;

	if ( h == 24) {
		h = 0;
	}

	if ( ms == 100) {
		s++;
		ms = 0;

		if (s == 60) {
			m++;
			s = 0;

			if ( m == 60) {
				h++;
				m = 0;
			}
		}
	}


}

}

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
#endif
#endif

	Timer aika(0, 30, 0, true);
	Timer ledi(0, 10, 0, true);
	Timer skooppi(0, 0, 300, true);

	// write code here that uses two timer objects
	// one is used to print current time every 30 seconds (use semihosting for printing)
	// second to switch led colour every 10 seconds
	// third to give a pulse on PIO0_10 every 300 ms
	// make the timer tick from main program - do not call object's methods from ISR

	// Otetaan käyttöön GPION 0,10
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 10, (IOCON_DIGMODE_EN));
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 10);

	// Force the counter to be placed into memory
	volatile static int i = 0 ;

	/* The sysTick counter only has 24 bits of precision, so it will
	   overflow quickly with a fast core clock. You can alter the
	   sysTick divider to generate slower sysTick clock rates. */
	Chip_Clock_SetSysTickClockDiv(1);

	/* A SysTick divider is present that scales the sysTick rate down
	   from the core clock. Using the SystemCoreClock variable as a
	   rate reference for the SysTick_Config() function won't work,
	   so get the sysTick rate by calling Chip_Clock_GetSysTickClockRate() */
	uint32_t sysTickRate = Chip_Clock_GetSysTickClockRate();

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(sysTickRate / TICKRATE_HZ1);

	while(1){

		// Enter an infinite loop, just incrementing a counter
		while(!flag) __WFI();
		flag = false;

		if(aika.Tick()){
			// Aika on loppu tässä
			// Tulosta aika semihostingilla
			//enter_critical();
			 printf("RealTimeClock after every 30secons.: %d.%d.%d\n", h,m,s);
		}

		if(ledi.Tick()){
			//Aika on loppu tässä
			Board_LED_Set(q, false);
			q++;
			Board_LED_Set(q, true);
			if (q == 2){
				q = 0;
			}
		}

		if(skooppi.Tick()){
			//Aika on loppu tässä
			// pulssi
			if (pulssi == 0) {
				Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
				pulssi = 1;
			}else{
				pulssi = 0;
				Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, false);
			}

		}
		i++ ;
	}
	return 0 ;
}
