#ifndef KEYS_H_
#define KEYS_H_
#include "main.h"

/* Configuration */
#define KEYS_NUM	  		(4)

//#define KEYS_USE_HAL_TICK
#define KEYS_USE_OS_TICK

#define KEYS_TICK_DIVIDER	(1)
#define KEYS_DEBOUNCE_TMR	(2		/ KEYS_TICK_DIVIDER)
#define KEYS_LONGCLICK_TMR	(16		/ KEYS_TICK_DIVIDER)
#define KEYS_REPEAT_TMR		(6		/ KEYS_TICK_DIVIDER)
/* End configuration ----------------------------------- */


typedef enum
{
	KEYS_NONE 			= 0,
	KEYS_CLICK 			= 1,
	KEYS_LONGCLICK 		= 2,
	KEYS_REPEAT 		= 3/*,
	KEYS_DOUBLECLICK 	= 4,
	KEYS_KEYUP 			= 5,
	KEYS_KEYDOWN 		= 6,
	KEYS_KEYERROR 		= 7*/
} KEYS_KEYSTROKE_E;

typedef enum
{
	KEYS_KEYTYPE_HI = 0,	// Active high
	KEYS_KEYTYPE_LO = 1,	// Active low
} KEYS_KEYTYPE_E;

typedef struct {
	GPIO_TypeDef  *gpio_port;
	uint16_t gpio_pin;
	uint16_t counter;
	union
	{
		uint8_t status_data;
		struct
		{
			uint8_t type			: 1;
			uint8_t debounced		: 1;
			uint8_t release			: 1;
			uint8_t repeat			: 1;
			uint8_t previous_val	: 1;
			uint8_t _reserved		: 3;
		};
	};
} KEYS_BUTTON_STATUS_T;

typedef struct {
	uint8_t key;
	uint8_t keystroke;
} KEYS_KEYEVENT_T;

int8_t Keys_AddKey(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, uint8_t type);
KEYS_KEYEVENT_T Keys_GetKeyEvent(void);
void Keys_Callback(void);

#endif /* KEYS_H_ */
