#include "keys.h"
#ifdef KEYS_USE_OS_TICK
#include "cmsis_os.h"
#endif

volatile static KEYS_BUTTON_STATUS_T Buttons[KEYS_NUM];
volatile static KEYS_KEYEVENT_T Event = {.key = 0, .keystroke = KEYS_NONE};	// todo event queue (fifo)

int8_t Keys_AddKey(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, uint8_t type)
{
	int8_t result = -1;

	for(uint8_t i = 0; i < KEYS_NUM; i++)
	{
		if(Buttons[i].gpio_port == NULL)
		{
			Buttons[i].gpio_port = gpio_port;
			Buttons[i].gpio_pin  = gpio_pin;
			Buttons[i].status_data = 0;
			Buttons[i].type 	 = type;
			Buttons[i].counter 	 = KEYS_DEBOUNCE_TMR;

			result = i;
			break;
		}
	}

	return result;
}

KEYS_KEYEVENT_T Keys_GetKeyEvent(void)
{
	KEYS_KEYEVENT_T result;
	result = Event;

	Event.keystroke = KEYS_NONE;
	//Event.key = 0;

	return result;
}

static inline void Keys_AddKeyEvent(uint8_t key, uint8_t keystroke)
{
	Event.key = key;
	Event.keystroke = keystroke;
}

static inline uint8_t Keys_GetPinValue(uint8_t key)
{
	uint8_t value = (HAL_GPIO_ReadPin(Buttons[key].gpio_port, Buttons[key].gpio_pin) == GPIO_PIN_SET);

	return Buttons[key].type ? !value : value;
}

#ifdef KEYS_USE_HAL_TICK		// Use HAL GetTick function
#define Keys_GetTick()     HAL_GetTick()
#elif defined KEYS_USE_OS_TICK	// Use CMSIS-RTOS SysTick function
#define Keys_GetTick()     osKernelSysTick()
#else	// Use internal tick counter
static volatile uint32_t _kbd_ticks;
static inline uint32_t Keys_GetTick(void)
{
	return ++_kbd_ticks;
}
#endif

// Call from Timer/SysTick ISR
// or from OS SysTick Hook
void Keys_Callback(void)
{
	uint8_t val;

	if((Keys_GetTick() & (KEYS_TICK_DIVIDER - 1))) return;		// every KEYS_TICK_DIVIDER interrupt served to save CPU time

	for(uint8_t i = 0; i < KEYS_NUM; i++)
	{
		if (Buttons[i].gpio_port == NULL) continue;			// only configured keys

		val = Keys_GetPinValue(i);
		if (val != Buttons[i].previous_val)
		{
			if (!Buttons[i].debounced && --Buttons[i].counter) continue;	// debounce
			Buttons[i].debounced = 1;
			if (val)
				Buttons[i].release = 1;
			Buttons[i].previous_val = val;
		} else {
			if (Buttons[i].debounced)
			{
				if (val)	// Key is down
				{
					if (Buttons[i].repeat) {
						if (!--Buttons[i].counter)
						{
							Keys_AddKeyEvent(i, KEYS_REPEAT);
							Buttons[i].counter = KEYS_REPEAT_TMR;
						}
					} else {
						if (++Buttons[i].counter >= KEYS_LONGCLICK_TMR)
						{
							Keys_AddKeyEvent(i, KEYS_LONGCLICK);
							Buttons[i].repeat  = 1;
							Buttons[i].counter = KEYS_REPEAT_TMR;
						}
					}
				} else { 	// Key is up
					if (Buttons[i].release)	// Key release
					{
						if (!Buttons[i].repeat)
							Keys_AddKeyEvent(i, KEYS_CLICK);
						else
							Buttons[i].repeat = 0;
						Buttons[i].release = 0;
					}
					Buttons[i].debounced = 0;
					Buttons[i].counter = KEYS_DEBOUNCE_TMR;
				}
			}
		}
	}
}
