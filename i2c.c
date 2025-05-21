#include <xc.h>

void init_i2c(void)
{
	/* Set SCL and SDA pins as inputs */
	TRISC3 = 1;
	TRISC4 = 1;
	/* Set I2C master mode */
	SSPCON1 = 0x28;

	SSPADD = 0x31;
	/* Use I2C levels, worked also with '0' */
	CKE = 0;
	/* Disable slew rate control  worked also with '0' */
	SMP = 1;
	/* Clear SSPIF interrupt flag */
	SSPIF = 0;
	/* Clear bus collision flag */
	BCLIF = 0;
}

void i2c_idle(void)
{
	while (!SSPIF); // check whether communication is going on or not
	SSPIF = 0;
}

void i2c_ack(void)
{
	if (ACKSTAT)
	{
		/* Do debug print here if required */
	}
}

void i2c_start(void)
{
	SEN = 1;
	i2c_idle();
}

void i2c_stop(void)      // to stop the communication
{
	PEN = 1;
	i2c_idle();
}

void i2c_rep_start(void)           //to repeat start
{
	RSEN = 1;
	i2c_idle();
}

void i2c_write(unsigned char data)
{
	SSPBUF = data;                // load the data to the buffer
	i2c_idle();
}

void i2c_rx_mode(void)         // enable the receive operation on the mc
{
	RCEN = 1;
	i2c_idle();
}

void i2c_no_ack(void)
{
	ACKDT = 1;                // mc sends proper ack if 0/ no ack if 1
	ACKEN = 1;                
}

unsigned char i2c_read(void)   // to initiate read operation 
{
	i2c_rx_mode();
	i2c_no_ack();

	return SSPBUF;
}