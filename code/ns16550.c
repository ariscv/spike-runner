#include <stdint.h>


#define NS16550_BASE		(0x10000000)
static inline uint8_t ns16550_in(uint32_t reg)
{
	return *(volatile uint8_t *) (NS16550_BASE + reg);
}

static inline void ns16550_out(uint32_t reg, uint8_t val)
{
	*(volatile uint8_t *) (NS16550_BASE + reg) = val;
}


#define NS16550_RBR		0
#define NS16550_THR		0
#define NS16550_DLL		0
#define NS16550_IER		1
#define NS16550_DLM		1
#define NS16550_FCR		2
#define NS16550_IIR		2
#define NS16550_LCR		3
#define NS16550_MCR		4
#define NS16550_LSR		5
#define NS16550_MSR		6
#define NS16550_SCR		7

#define NS16550_LSR_DR		0x01	/* Data ready */
#define NS16550_LSR_OE		0x02	/* Overrun */
#define NS16550_LSR_PE		0x04	/* Parity error */
#define NS16550_LSR_FE		0x08	/* Framing error */
#define NS16550_LSR_BI		0x10	/* Break */
#define NS16550_LSR_THRE	0x20	/* Xmit holding register empty */
#define NS16550_LSR_TEMT	0x40	/* Xmitter empty */
#define NS16550_LSR_ERR		0x80	/* Error */


void _serinit(void)
{
#if USE_NILE4_SERIAL
	ns16550_out(NS16550_LCR, 0x80);
	ns16550_out(NS16550_DLM, 0x00);
	ns16550_out(NS16550_DLL, 0x36);	/* 9600 baud */
	ns16550_out(NS16550_LCR, 0x00);
	ns16550_out(NS16550_LCR, 0x03);
	ns16550_out(NS16550_FCR, 0x47);
#else
	/* done by PMON */
#endif
}

void _putc(char c)
{
	while (!(ns16550_in(NS16550_LSR) & NS16550_LSR_THRE));
	ns16550_out(NS16550_THR, c);
	if (c == '\n') {
		while (!(ns16550_in(NS16550_LSR) & NS16550_LSR_THRE));
		ns16550_out(NS16550_THR, '\r');
	}
}

void _puts(const char *s)
{
	char c;
	while ((c = *s++))
		_putc(c);
}

char _getc(void)
{
	while (!(ns16550_in(NS16550_LSR) & NS16550_LSR_DR));
	return ns16550_in(NS16550_RBR);
}

int _testc(void)
{
	return (ns16550_in(NS16550_LSR) & NS16550_LSR_DR) != 0;
}
