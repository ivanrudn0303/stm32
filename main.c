#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f051x8.h"

#define allmod 0xef1

void SystemClock_Config(void);

int tick = 0, cond = 0, button_tick = 0;
int time = 19 * 3600 + 22 * 60 + 50;
int num_shaf[10] = {0x3FF,0x30F, 0x5EF, 0x7AF, 0x71F, 0x7BE, 0x7FE, 0x32F, 0x7FF, 0x7BF};
char state = 0, button_reg = 0, timer[4] = {0};

main(void) {
	int time_reg, time_push = 0, i = 0, time_alarm = 19 * 3600 + 23 * 60;
	SystemClock_Config();
	NVIC_SetPriority(SysTick_IRQn, 1);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	buttoninit();


	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_3, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_11, LL_GPIO_MODE_OUTPUT);	

	while (1)
	{
		if(state)
		{
			if(tick >= 1000)
			{
				time ++;
				tick = 0;
				timer[0] = time / 36000;
				timer[1] = (time / 3600) % 10;
				timer[2] = (time % 3600) / 600;
				timer[3] = ((time % 3600) / 60) % 10;
				if(time == time_alarm)
				{
					time_alarm = 25 * 3600;
					budit(5);
				}
			}
			if(((button_tick + 150) % 1000 == tick) && button_reg)
			{
				if(!time_push)
					time_push = time;
				i ++;
				button_reg = 0;
			}
			if((time - time_push > 1) && time_push)
			{
				if(i == 2)
					change_time();
				if(i == 3)
					set_alarm(&time_alarm);
				i = 0;
				time_push = 0;
			}
			if(time >= 24 * 3600)
				time = 0;
			if(cond == 0)
			{
				time_reg = num_shaf[timer[0]];
				time_reg ^= 0x8;
				WRITE_REG(GPIOC->ODR, time_reg);
				cond ++;
			}
			else
				if(cond == 1)
				{
					time_reg = num_shaf[timer[1]];
					if(tick < 500)
						time_reg += 2048;
					time_reg ^= 4;
					WRITE_REG(GPIOC->ODR, time_reg);
					cond ++;
				}
				else
					if(cond == 2)
					{
						time_reg = num_shaf[timer[2]];
						time_reg ^= 2;
						WRITE_REG(GPIOC->ODR, time_reg);
						cond ++;	
					}
					else
					{
						time_reg = num_shaf[timer[3]];
						time_reg ^= 0x100;
						WRITE_REG(GPIOC->ODR, time_reg);
						cond = 0;
					}
			state = 0;
		}

	}
}

/**
  * System Clock Configuration
  * The system Clock is configured as follow :
  *    System Clock source            = PLL (HSI/2)
  *    SYSCLK(Hz)                     = 48000000
  *    HCLK(Hz)                       = 48000000
  *    AHB Prescaler                  = 1
  *    APB1 Prescaler                 = 1
  *    HSI Frequency(Hz)              = 8000000
  *    PLLMUL                         = 12
  *    Flash Latency(WS)              = 1
  */


void
SystemClock_Config() {
        /* Set FLASH latency */
        LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

        /* Enable HSI and wait for activation*/
        LL_RCC_HSI_Enable();
        while (LL_RCC_HSI_IsReady() != 1);

        /* Main PLL configuration and activation */
        LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
                                    LL_RCC_PLL_MUL_12);

        LL_RCC_PLL_Enable();
        while (LL_RCC_PLL_IsReady() != 1);

        /* Sysclk activation on the main PLL */
        LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
        LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
        while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

        /* Set APB1 prescaler */
        LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

        /* Set systick to 1ms */
        SysTick_Config(48000000/1000);

        /* Update CMSIS variable (which can be updated also
         * through SystemCoreClockUpdate function) */
        SystemCoreClock = 48000000;
}

void change_time(void)
{
	int i = 0, time_push = 0, cond = 0, time_reg = 0;
	while( i != 3)
	{
		if(state)
		{
			if(tick >= 1000)
			{
				tick = 0;
				time ++;
			}
			if(((button_tick + 150) % 1000 == tick) && button_reg)
			{
				if(!time_push)
					time_push = time;
				i ++;
				button_reg = 0;
			}
			if((time - time_push > 1) && time_push)
			{
				if(i == 1)
					timer[cond] ++;
				if(i == 2)
					cond ++;
				if(cond == 4)
					cond = 0;
				i = 0;
				time_push = 0;
			}
			time_reg = num_shaf[timer[cond] % 10];
			if(cond == 0)
				time_reg ^= 8;
			else
				if(cond == 1)
					time_reg ^= 4;
				else
					if(cond == 2)
						time_reg ^= 2;
					else
						time_reg ^= 0x100;
			WRITE_REG(GPIOC->ODR, time_reg);
			state = 0;
		}
	}
	time = (((timer[0] % 10) * 10 + (timer[1] % 10)) * 60 + ((timer[2] % 10) * 10) + (timer[3] % 10)) * 60;
}

void set_alarm(int *time_alarm)
{
	int i = 0, time_push = 0, cond = 0, time_reg = 0;
	timer[0] = (*time_alarm) / 36000;
	timer[1] = (*time_alarm / 3600) % 10;
	timer[2] = ((*time_alarm) % 3600) / 600;
	timer[3] = (((*time_alarm) % 3600) / 60) % 10;

	while( i != 3)
	{
		if(state)
		{
			if(tick >= 1000)
			{
				tick = 0;
				time ++;
			}
			if(((button_tick + 150) % 1000 == tick) && button_reg)
			{
				if(!time_push)
					time_push = time;
				i ++;
				button_reg = 0;
			}
			if((time - time_push > 1) && time_push)
			{
				if(i == 1)
					timer[cond] ++;
				if(i == 2)
					cond ++;
				if(cond == 4)
					cond = 0;
				i = 0;
				time_push = 0;
			}
			time_reg = num_shaf[timer[cond] % 10];
			if(cond == 0)
				time_reg ^= 8;
			else
				if(cond == 1)
					time_reg ^= 4;
				else
					if(cond == 2)
						time_reg ^= 2;
					else
						time_reg ^= 0x100;
			WRITE_REG(GPIOC->ODR, time_reg);
			state = 0;
		}
	}
	*time_alarm = (((timer[0] % 10) * 10 + (timer[1] % 10)) * 60 + ((timer[2] % 10) * 10) + (timer[3] % 10)) * 60;
}

void budit(int raz)
{
	while(raz)
	{
		if(tick >= 1000)
		{
			time ++;
			tick = 0;
			raz --;
		}
		if(tick == 5)
			WRITE_REG(GPIOC->ODR, allmod);
		if(tick == 500)
			WRITE_REG(GPIOC->ODR, 0);
	}
}

void buttoninit(void) {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
        LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
        LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
        LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);
        NVIC_EnableIRQ(EXTI0_1_IRQn);
        NVIC_SetPriority(EXTI0_1_IRQn, 16);
}

void
NMI_Handler(void) {
}

void
HardFault_Handler(void) {
        while (1);
}

void
SVC_Handler(void) {
}

void
PendSV_Handler(void) {
}

void
SysTick_Handler(void) {
	tick++;
	state = 1;
}

void EXTI0_1_IRQHandler(void)
{
	if(button_reg == 0)
	{
		button_reg = 1;
		button_tick = tick - 1;
	}
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);	
}
