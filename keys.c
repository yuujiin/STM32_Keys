#include "keys.h"

volatile static keys_button_status_t Buttons[KEYS_NUM];

/* Configure a button.
 * Parameters: GPIO port & pin, key type (active high/low)
 * Return: key id >= 0 if success, -1 for error
 */
int8_t Keys_AddKey(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, uint8_t type) {
	int8_t result = -1;

	for(uint8_t i = 0; i < KEYS_NUM; i++) {
		if(Buttons[i].gpio_port == NULL) {
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

static inline void Keys_AddKeyEvent(uint8_t key, uint8_t keystroke) {
	keys_keyevent_t event;
	event.key = key;
	event.keystroke = keystroke;

	osMessagePut(KEYS_QUEUE_HANDLE, event.v, KEYS_QUEUE_TIMEOUT);
}

static inline uint8_t Keys_GetPinValue(uint8_t key) {
	uint8_t value = (HAL_GPIO_ReadPin(Buttons[key].gpio_port, Buttons[key].gpio_pin) == GPIO_PIN_SET);

	return Buttons[key].type ? !value : value;
}

/* To be called from OS SysTick Hook
 */
void Keys_Callback(void) {
	uint8_t val;

	if((osKernelSysTick() & (KEYS_TICK_DIVIDER - 1))) return;		// every KEYS_TICK_DIVIDER interrupt served to save CPU time

	for(uint8_t i = 0; i < KEYS_NUM; i++) {
		if (Buttons[i].gpio_port == NULL) continue;			// only configured keys

		val = Keys_GetPinValue(i);
		if (val != Buttons[i].prev_val) {
			if (!Buttons[i].debounced && --Buttons[i].counter) continue;	// debounce
			Buttons[i].debounced = 1;
			if (val)
				Buttons[i].release = 1;
			Buttons[i].prev_val = val;
		} else {
			if (Buttons[i].debounced) {
				if (val) {	// Key is down
					if (Buttons[i].repeat) {
						if (!--Buttons[i].counter)
						{
							Keys_AddKeyEvent(i, KEYS_REPEAT);
							Buttons[i].counter = KEYS_REPEAT_TMR;
						}
					} else {
						if (++Buttons[i].counter >= KEYS_LONGCLICK_TMR) {
							Keys_AddKeyEvent(i, KEYS_LONGCLICK);
							Buttons[i].repeat  = 1;
							Buttons[i].counter = KEYS_REPEAT_TMR;
						}
					}
				} else { 	// Key is up
					if (Buttons[i].release) {	// Key release
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
