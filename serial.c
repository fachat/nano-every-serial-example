

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

#define BAUDRATE	38400
#define	BAUD_FROM_RATE(RATE)  ((4UL * (F_CPU))/(RATE))

uint8_t p[] = "The quick brown fox jumps over the lazy dog\n\r";

int main(void)
{

	// setup 16 MHz clock clk_per
	// clk_per source is 16/20mhz osc
	_PROTECTED_WRITE((CLKCTRL.MCLKCTRLA), CLKCTRL_CLKSEL_OSC20M_gc);
	// disable pre-scaler
	_PROTECTED_WRITE((CLKCTRL.MCLKCTRLB), (0 << CLKCTRL_PEN_bp));	


	// port MUX
	PORTMUX.USARTROUTEA = (PORTMUX.USARTROUTEA & 0x3f) | 0x40;
	// TX 
	PORTB.PIN4CTRL = PORT_PULLUPEN_bm;
	PORTB.DIR |= PIN4_bm;
	PORTB.DIR &= ~PIN5_bm;

	// uart_init
	USART3.BAUD = BAUD_FROM_RATE(BAUDRATE);
	USART3.CTRLC = USART_SBMODE_2BIT_gc | USART_CHSIZE_8BIT_gc;
	USART3.CTRLA = 0;

	// uart on, TX and RX
	USART3.CTRLB = USART_TXEN_bm | USART_RXEN_bm;

	// send a test string
	int i = 0;
	while (p[i] != 0) {

		while (!(USART3.STATUS & USART_DREIF_bm))
			;

		USART3.TXDATAL = p[i];
		i++;
	}

	// receive a byte and echo with bit 0 inverted
	while (1) {

		while (!(USART3.STATUS & USART_RXCIF_bm))
			;

		// there is a char
		uint8_t c = USART3.RXDATAL;

		// modify to distinguish from echo
		c += 1;

		while (!(USART3.STATUS & USART_DREIF_bm))
			;
		// and send
		USART3.TXDATAL = c;
	}

	return 0;
}

