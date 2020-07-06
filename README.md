# Button reader library for STM32 with RTOS
Button debounce, long and repeated click events.
Using STM32 HAL & CMSIS-RTOS API.

## How to use
* Configure parameters in keys.h:
	* KEYS_NUM - maximum number of configured keys
	* KEYS_QUEUE_HANDLE - RTOS message queue handle for events (data size = 16 bits)
	* KEYS_QUEUE_TIMEOUT - time to wait when queue is full
* Add a call to Keys_Callback() from RTOS SysTick hook
* Configure buttons using Keys_AddKey()
	* Specify GPIO Port & Pin
	* Select button type: active low / active high
* Read key events from message queue

## Code example
	#include "keys.h"
	/* Initialization */
	void Init(void) {
		key1 = Keys_AddKey(KEY1_GPIO_Port, KEY1_Pin, KEYS_KEYTYPE_LO);	// key1 = 0
		key2 = Keys_AddKey(KEY2_GPIO_Port, KEY2_Pin, KEYS_KEYTYPE_LO);	// key2 = 1
	}
	/* OS SysTick hook */
	void vApplicationTickHook(void) {
		Keys_Callback();
	}
	/* OS Task */
	void KeyTask(void const * argument) {
		osEvent event;
		keys_keyevent_t keyevent;
		while (1) {
			event = osMessageGet(keyeventHandle, osWaitForever);
			if (event.status == osEventMessage) {
				keyevent.v = event.value.v;				
				switch (keyevent.key) {
				case key1:
					if (keyevent.keystroke == KEYS_CLICK) {
						// action for single click
						// menu_next();
					} else if (keyevent.keystroke == KEYS_LONGCLICK) {
						// action for long click
						// menu_exit();
					}
					break;
				case key2:
					// action for any type of event
					// some_param++;
					break;
				}
			}
		}
	}