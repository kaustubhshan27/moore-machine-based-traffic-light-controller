#include "TM4C123.h"

#define goN			0		//Go - cars coming from north
#define waitN			1		//Wait - cars coming from north
#define goW			2		//Go - cars coming from west
#define waitW			3		//Wait - cars coming from west
#define goPed			4		//Go - pedestrian crossing
#define hurryPed		5		//Hurry Up - pedestrian crossing
#define stopPed			6		//Stop - pedestrian crossing

struct State
{
	unsigned char traffic_light_out;				//traffic light output, pedestrian crossing output
	unsigned long time;											//state wait time (multiples of 10ms)
	unsigned long next[8];									//next state depending on input and current state
};

typedef struct State STATE;

//declaring and initializing struct mapping of finite state machine states
STATE FSM[7] = 
	{
		{0x61, 400, {goN, waitN, waitN, waitN, goN, waitN, waitN, waitN}},
		{0x51, 200, {goW, goPed, goW, goW, goW, goPed, goW, goW}},
		{0x4C, 400, {goW, waitW, goW, waitW, waitW, waitW, waitW, waitW}},
		{0x4A, 200, {goN, goPed, goN, goPed, goN, goPed, goN, goPed}},
		{0x89, 400, {goPed, goPed, hurryPed, hurryPed, hurryPed, hurryPed, hurryPed, hurryPed}},
		{0x89, 0, {stopPed, stopPed, stopPed, stopPed, stopPed, stopPed, stopPed, stopPed}},
		{0x49, 400, {goN, goN, goW, goW, goN, goN, goN, goN}}
	};		// 7 FSM states. Based on Moore Model

void systick_init(void);									//initialising systic timer for state wait time
void portB_init(void);										//traffic light signal outputs
void portD_init(void);	  								//button inputs - north road, west road, pedestrian crossing
void wait_time(unsigned long value);			//delay time (multiples of 10ms)
void toggle_pedestrian(void);							//toggle pedestrian green light for "hurryPed" state

int main(void)
{
	unsigned long current_state = goN;
	unsigned char input = 0x00;
	
	portB_init();
	portD_init();
	
	while(1)
	{
		GPIOB->DATA = FSM[current_state].traffic_light_out;					//traffic light output, pedestrian crossing light output
		
		if(current_state == hurryPed)																//if in "hurryPed" state toggle green pedestrian crossing light
			toggle_pedestrian();
		else																												//wait time for the current state
			wait_time(FSM[current_state].time);
		
		input = GPIOD->DATA;																				//taking input from buttons - PD2, PD1, PD0
		current_state = FSM[current_state].next[input];							//next state = f(current state, input)
	}
	
	return 0;
}

void systick_init(void)
{
	SysTick->CTRL = 0x04;					//enable = 0, clk_src = 1 (16 MHz Bus Clock)
	SysTick->LOAD = 160000 - 1;		//10ms count
	SysTick->VAL = 0x00;					//to clear by writing to the current value register
	SysTick->CTRL |= (1 << 0);		//enable and start timer
}

void portB_init(void)
{
	SYSCTL->RCGCGPIO |= (1 << 1);
	GPIOB->CR |= 0xFF;
	GPIOB->DEN |= 0xFF;
	GPIOB->AFSEL &= ~(0xFF);
	GPIOB->PCTL = 0x00000000;
	GPIOB->DIR |= 0xFF;					//output pins - PB5, PB4, PB3, PB2, PB1, PB0 = traffic lights
}

void portD_init(void)
{
	SYSCTL->RCGCGPIO |= (1 << 3);
	GPIOD->CR |= 0x07;
	GPIOD->DEN |= 0x07;
	GPIOD->AFSEL &= ~(0x07);
	GPIOD->PCTL = 0x00000000;
	GPIOD->DIR |= 0x00;					//input pins - PD2, PD1, PD0 = button inputs
}

void wait_time(unsigned long value)
{
	//Systick timer initialization 
	systick_init();
	
	for(unsigned long loop = 0; loop < value; loop++)	//total delay = value * 10ms	
	{
		while((SysTick->CTRL & 0x10000) != 0x10000);	//inner loops gives 10ms delay
	}
}

void toggle_pedestrian(void)
{
	systick_init();
	
	for(unsigned long count = 0; count < 3; count++)
	{
		GPIOE->DATA = 0x02;
		for(unsigned long loop = 0; loop < 100; loop++)	//total delay = 1s	
		{
			while((SysTick->CTRL & 0x10000) != 0x10000);	//inner loops gives 10ms delay
		}
	
		GPIOE->DATA = 0x00;
		for(unsigned long loop = 0; loop < 100; loop++)	//total delay = 1s	
		{
			while((SysTick->CTRL & 0x10000) != 0x10000);	//inner loops gives 10ms delay
		}
	}
}
