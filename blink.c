

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>


int main(void)
{

	// setup 16 MHz clock clk_per
	// clk_per source is 16/20mhz osc
	_PROTECTED_WRITE((CLKCTRL.MCLKCTRLA), CLKCTRL_CLKSEL_OSC20M_gc);
	// disable pre-scaler
	_PROTECTED_WRITE((CLKCTRL.MCLKCTRLB), (0 << CLKCTRL_PEN_bp));	


	// port PB4 as output (TX pin)
	PORTB.DIR |= PIN4_bm;

	// Pull-up resistor 
	PORTB.PIN4CTRL = PORT_PULLUPEN_bm;

	while (1)
	{
		PORTB.OUT |= PIN4_bm;
		_delay_ms(100);
		PORTB.OUT &= ~PIN4_bm;
		_delay_ms(100);
	}

}

