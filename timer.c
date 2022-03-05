
// see also http://ww1.microchip.com/downloads/en/AppNotes/TB3217-Getting-Started-with-TCA-90003217A.pdf

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>

#define	RATE_PER_SEC	0

//#define PERIOD_VALUE    (((F_CPU) / (RATE_PER_SEC)) / 256)

#define	PERIOD_VALUE	0xffff

#define	PERVAL_FOR_INTSECS_256PRESCALE(INT)	((((INT)*(F_CPU))/256)-1)

ISR(TCA0_OVF_vect) {
	// invert LED
	PORTB.OUT ^= PIN4_bm;

	// disable Interrupt, so it does not trigger again immediately
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

int main(void)
{

	// setup 16 MHz clock clk_per
	// clk_per source is 16/20mhz osc
	_PROTECTED_WRITE((CLKCTRL.MCLKCTRLA), CLKCTRL_CLKSEL_OSC20M_gc);
	// disable pre-scaler
	_PROTECTED_WRITE((CLKCTRL.MCLKCTRLB), (0 << CLKCTRL_PEN_bp));	

	// setup timer TCA
	//
	// use periodic interrupt mode

	// reset the timer
	TCA0.SINGLE.CTRLA = 0;		// stop
	TCA0.SINGLE.CTRLESET = 0xc0;	// reset

	/* set Normal mode */
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    
	/* disable event counting */
	TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    
	/* set the period */

	// TCA_per = ((time(s) * f_clk(Hz)) / TCA_prescaler) - 1
	// f_clk = F_CPU = 16000000
	// time(s) = 10ms = 0.01s
	// TCA_prescaler = 256
	// -> TCA_per = 624
	TCA0.SINGLE.PER = PERVAL_FOR_INTSECS_256PRESCALE(0.2);
    
	/* Timer A pre-scaler is 256, and enable timer */
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc
		| TCA_SINGLE_ENABLE_bm;

	/* enable overflow interrupt */
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;


	// port PB4 as output (TX pin)
	PORTB.DIR |= PIN4_bm;

	// Pull-up resistor 
	PORTB.PIN4CTRL = PORT_PULLUPEN_bm;

	// enable interrupts
	sei();

	// wait
	while (1);

	return 0;
}

