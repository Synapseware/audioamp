/*
 * audioamp.h
 *
 * Created: 8/29/2012 8:17:26 PM
 *  Author: matthew
 */ 


#ifndef AUDIOAMP_H_
#define AUDIOAMP_H_


#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#include <libs/events/events.h>

#define SAMPLE_RATE 16000


#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))


//----------------------------------------------------------------------------------------------
// Serial LED
#define LEDS_PORT			PORTD
#define LEDS_DDR			DDRD
#define LEDS_RED_LED		PORTD5
#define LEDS_GRN_LED		PORTD6
#define LEDS_BLU_LED		PORTD7

#define leds_en()			LEDS_DDR |= (1<<LEDS_RED_LED) | (1<<LEDS_GRN_LED) | (1<<LEDS_BLU_LED)

#define AUD_CTRL_PORT		PORTD
#define AUD_CTRL_DDR		DDRD
#define AUD_CTRL_PIN		PORTD0
#define aud_ctrl_en()		AUD_CTRL_DDR |= (1<<AUD_CTRL_PIN)

#define CHANNEL_BLU			0
#define CHANNEL_GRN			1


/*
led +	Red		Green		Blue
------	------	------		------
Purple	Grey	White		Black


*/


//----------------------------------------------------------------------------------------------
const static uint8_t SLEEPY_EYES[] PROGMEM = {
	20, 32, 40, 48, 64, 96, 127, 160, 192, 240, 192, 160, 140, 130, 96, 64, 48, 40, 32, 20, 20
};
#define SLEEPY_EYES_LEN sizeof(SLEEPY_EYES)/sizeof(uint8_t)



#endif /* AUDIOAMP_H_ */