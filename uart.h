#ifndef SCI_H
#define SCI_H

#define RX_PIN					TRISC7
#define TX_PIN					TRISC6

void init_uart(void);
void putch(unsigned char byte);      // TO TRANSMIT 1 BYTE OF DATA
int puts(const char *s);             //TO TRANSMIT A STRING
unsigned char getch(void);          // TO RECEIVE THE 1 BYTE OF DATA
unsigned char getche(void);         // TO RECEIVE 1 BYTE AND TRANSMIT 1 bYTE

#endif
