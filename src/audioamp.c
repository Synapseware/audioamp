/*
 * audioamp.c
 *
 * Created: 8/29/2012 7:47:04 PM
 *  Author: matthew
 */ 
#include "audioamp.h"

volatile static uint8_t		_samples[2];
volatile static uint8_t		_lastChannel	= 0;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// sets up a sample request for a specific channel (state = channel #)
void adcSampleHandler(uint8_t state)
{
	_lastChannel++;
	if (_lastChannel > 1)
		_lastChannel = 0;

	// or-in the desired channel
	ADMUX = (ADMUX & 0xE0) | _lastChannel;

	// start a conversion
	ADCSRA |= (1<<ADSC);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// un-mute the amp
void ampOnHandler(uint8_t state)
{
	AUD_CTRL_PORT	&= ~(1<<AUD_CTRL_PIN);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// initialize the application
void init(void)
{
	_lastChannel			= 0;
	_samples[CHANNEL_BLU]	= 0;
	_samples[CHANNEL_GRN]	= 0;


	// enable ADC & timer1
	PRR =	(1<<PRTWI) |
			(1<<PRTIM2) |
			(0<<PRTIM1) |
			(1<<PRTIM0)	|
			(0<<PRADC);
	
	// prepare ADC
	ADMUX =	(0<<REFS1) |
			(1<<REFS0) |
			(1<<ADLAR);			// left adjust (ADCH has the most-significant 8 bits)

	// setup
	ADCSRA = (1<<ADEN) |
			(0<<ADSC) |
			(0<<ADATE) |
			(1<<ADIF) |
			(1<<ADIE) |
			(1<<ADPS2) |	// prescale should be ~160 (20mHz/125kHz = 160)
			(0<<ADPS1) |
			(1<<ADPS0);

	// setup
	ADCSRB = (0<<ACME) |
			(0<<ADTS2) |
			(0<<ADTS1) |
			(0<<ADTS0);

	// disable digial inputs
	DIDR0 &= ~((1<<ADC1D) | (1<<ADC0D));

	// setup the input channels
	DDRC &= ~((1<<ADC0D) | (1<<ADC1D));

	// setup the LEDs
	leds_en();

	// setup the amp mute	
	aud_ctrl_en();
	AUD_CTRL_PORT	|= (1<<AUD_CTRL_PIN);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// establish event timer & handler
	// timer1, event timer
	// Set CTC mode (Clear Timer on Compare Match) (p.133)
	// Have to set OCR1A *before anything else*, otherwise it gets reset to 0!
	OCR1A	=	(F_CPU / SAMPLE_RATE);
	TCCR1A	=	0;
	TCCR1B	=	(1<<WGM12) |	// CTC
				(1<<CS10);
	TIMSK1	=	(1<<OCIE1A);

	setTimeBase(SAMPLE_RATE);
	
	sei();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
volatile static uint8_t _idx = 0;
volatile static uint8_t _val = 0;
void ledPWMHandler(eventState_t state)
{
	if (_idx == 0)
	{
		// turn on the LEDs
		LEDS_PORT &= ~((1<<LEDS_RED_LED) | (1<<LEDS_GRN_LED) | (1<<LEDS_BLU_LED));
	}
	else
	{
		// turn off the LEDs according to their channel brightness values
		// red first, which follows the sleepy-eye pattern from ROM
		if (_idx >= _val)
			LEDS_PORT |= (1<<LEDS_RED_LED);

		// green channel
		if (_idx >= _samples[CHANNEL_GRN])
			LEDS_PORT |= (1<<LEDS_GRN_LED);

		// blue channel
		if (_idx >= _samples[CHANNEL_BLU])
			LEDS_PORT |= (1<<LEDS_BLU_LED);
	}
	_idx+=2;
}

volatile uint8_t _i = 0;
void nextSleepyEyeHandler(eventState_t state)
{
	_i++;
	if (_i >= SLEEPY_EYES_LEN)
		_i = 0;
}
void slideToSleepyEyeHandler(eventState_t state)
{
	uint8_t val = pgm_read_byte(&SLEEPY_EYES[_i]);
	if (_val > val)
		_val--;
	else if (_val < val)
		_val++;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// main func
int main(void)
{
	init();
	
	// run as fast as possible
	registerHighPriorityEvent(ledPWMHandler, 0, 0);

	// move to the next sleepy-eye value
	registerEvent(nextSleepyEyeHandler, SAMPLE_RATE / 4, 0);

	// slide-up/slide-down to current sleepy-eye value
	registerEvent(slideToSleepyEyeHandler, SAMPLE_RATE / 100, 0);

	// subscribe to a time-based ACD sampling
	registerHighPriorityEvent(adcSampleHandler, SAMPLE_RATE / 50, 0);

	registerOneShot(ampOnHandler, SAMPLE_RATE, 0);

	// main program loop
    while(1)
    {
        eventsDoEvents();
    }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// ADC callback
ISR(ADC_vect)
{
	_samples[_lastChannel] = ADCH;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - -
// times the events
ISR(TIMER1_COMPA_vect)
{
	eventSync();
}
