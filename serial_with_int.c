

#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BAUDRATE	38400
#define	BAUD_FROM_RATE(RATE)  ((4UL * (F_CPU))/(RATE))


#define BUFFER_SIZE             128
#define BUFFER_SIZE_MASK        0x7f
#define BUFFER_NEXT(p)          ((p + 1) & BUFFER_SIZE_MASK)


// send ring buffer
volatile int8_t         txbuf[BUFFER_SIZE];
volatile uint8_t        tx_wp;
volatile uint8_t        tx_rp;

// receive ring buffer
volatile int8_t         rxbuf[BUFFER_SIZE];
volatile uint8_t        rx_wp;
volatile uint8_t        rx_rp;
volatile uint8_t        rx_err;


// enable (send) data register empty interrupt, called when needed
static inline void _uart_enable_sendisr() {
	USART3.CTRLA |= USART_DREIE_bm;
}

// disable (send) data register empty interrupt, called when needed
static inline void _uart_disable_sendisr() {
	USART3.CTRLA &= ~USART_DREIE_bm;
}


/**
 * when returns true, there is space in the uart send buffer,
 * so uarthw_send(...) can be called with a byte to send
 */
int8_t uarthw_can_send() {
        // check for wrap-around to read pointer
        uint8_t tmp = BUFFER_NEXT(tx_wp);
        return (tmp != tx_rp) ? 1 : 0;
}

/**
 * submit a byte to the send buffer
 */
void uarthw_send(int8_t data) {
        txbuf[tx_wp] = data;
        tx_wp = BUFFER_NEXT(tx_wp);
        _uart_enable_sendisr();
}

/**
 * receive a byte from the receive buffer
 * Returns -1 when no byte available
 */
int16_t uarthw_receive() {
        if (rx_wp == rx_rp) {
                // no data available
                return -1;
        }
        int16_t data = 0xff & rxbuf[rx_rp];
        rx_rp = BUFFER_NEXT(rx_rp);
        return data;
}


/**
 * interrupt for data send register empty
 */
ISR(USART3_DRE_vect) {

        if (tx_wp != tx_rp) {
                USART3.TXDATAL = txbuf[tx_rp];

                tx_rp = BUFFER_NEXT(tx_rp);
        }

        if (tx_wp == tx_rp) {
                // no more data available
                // Disable ISR
		_uart_disable_sendisr();
        }
}


/**
 * interrupt for received data
 */
ISR(USART3_RXC_vect)
{
        rxbuf[rx_wp] = USART3.RXDATAL;

        uint8_t tmp = BUFFER_NEXT(rx_wp);
        if (tmp == rx_rp) {
                // buffer overflow
                rx_err++;
                //led_on();
        } else {
                rx_wp = tmp;
        }

	// LED - optional, for debugging
	//PORTF_OUT ^= (1 << 5);	
}

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
	// RX
	PORTB.DIR &= ~PIN5_bm;

	// LED - optional, for debugging
	//PORTF.DIR |= PIN5_bm;

	// uart_init
	USART3.BAUD = BAUD_FROM_RATE(BAUDRATE);
	USART3.CTRLC = USART_SBMODE_2BIT_gc | USART_CHSIZE_8BIT_gc;
	USART3.CTRLA = 0;

	// uart on, TX and RX
	USART3.CTRLB = USART_TXEN_bm | USART_RXEN_bm;

	// enable receive interrupt
	USART3.CTRLA |= USART_RXCIE_bm;

	// enable global interrupts
	sei();

	// echo loop
	int16_t c;
	while (1) {
		while ((c = uarthw_receive()) == -1)
			;

		while (!uarthw_can_send())
			;
		uarthw_send(c & 0xff);
	}

	return 0;
}

