/*
 *
 * Created: 07/04/2023 18.11.05
 * Author : Rasmus Salmi
 *
 *This program reads servo motor position from virtual terminal, potentiometer
 *or thermistor, which is also potentiometer in the simulation. The position of
 *servo motor can be turned by sending '+' or '-' characters from virtual terminal.
 *Led shows the position of servo motor by how bright it is.
 *
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define BAUD 9600 // Baud rate
#define MYUBRR F_CPU/16/BAUD-1 // USART baud rate register

#define SERVO 0
#define TEMPERATURE 1

#define SYSTEM_SLEEP (PIND & (1<<PIND2)) == 0

//virtual terminal info
volatile char info[] = "Servo:  , Temperature:  ";
volatile unsigned char dataFromVirtualTerminal;

unsigned int potentiometerTemperature = 0;
unsigned int potentiometerServo = 0;
unsigned int virtualTerminalsetting = 0;
unsigned int servoPosition = 0;

void USARTtransmit(unsigned char data);

/*Initialization of USART data transfer*/
void initUSART(unsigned int ubrr)
{
	//Set baud rate
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)(ubrr);
	
	//Enable receiver and transmitter
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
	
	//Enable (transfer) data register empty and receive complete interrupts
	UCSR0B |= (1<<RXCIE0);
}

/*Initialization of on/off switch*/
void initOnOffSwitch(void)
{
	//Setting switch pin (PD2) as input
	DDRD &= ~(1<< DDD2);
	
	//Activating weak pull-up on pin PD2
	PORTD |= (1<<PORTD2);
	
	//Enabling pin change interrupt in port D
	PCICR |= (1<<PCIE2);
	
	//Enabling pin change interrupt in pin PD2
	PCMSK2 |= (1<<PCINT18);
}

/*Initialization of AD channels PC0 (potentiometer) and PC1 (temperature)*/
void initADC(void)
{
	//Init AD channels PC0 and PC1
	//Disable channel input buffers
	DIDR0 |= (1<<ADC0D) | (1<<ADC1D);
	
	//Write 1 to corresponding output pins to minimize power consumption
	PORTC |= (1<<PORTC0) | (1<<PORTC1);
	
	//Choose Vcc 5v reference
	ADMUX |= (1<<REFS0);
	
	//Using only high byte of the result
	ADMUX |= (1<<ADLAR);
	
	//Clock prescaler 128, AD-clock 125 kHz -> conversion time 104 us
	ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
	
	//Enable AD-converter in Power Reduction Register
	PRR &= ~(1<<PRADC);
	
	//Enable AD-converter in ADCSRA
	ADCSRA |= (1<<ADEN);
}

/*Initialization of PWM with timer 0 for servo position control*/
void initServoPWM(void)
{
	//Set PWM output pin
	DDRD |= (1<<DDD6);
	
	//Select fast PWM mode
	TCCR0A |= (1<<WGM01) | (1<<WGM00);
	
	//Pin state is cleared on compare match and set on overflow
	TCCR0A |= (1<<COM0A1) | (1<<COM0B1);
	
	//Prescaling by 256 -> 62.5 kHz
	TCCR0B |= (1<<CS02);
	
	//Initialize counter
	TCNT0 = 0;
	
	//Initialize compare value
	OCR0A = 0;
}

/*Initialization of PWM with timer 2 for led brightness control*/
void initLedPWM(void)
{
	//Set PWM output pin
	DDRD |= (1<<DDD3);
	
	//Select fast PWM mode
	TCCR2A |= (1<<WGM21) | (1<<WGM20);
	
	//Pin state is cleared on compare match and set on overflow
	TCCR2A |= (1<<COM2B1); 
	
	//No prescaling
	TCCR2B |= (1<<CS20);
	
	//Initialize counter
	TCNT2 = 0;
	
	//Initialize compare value
	OCR2B = 0;
}

/*Init 16bit timer1 to interrupt every 2 second for data transfer*/
void initTimer1(void)
{
	//CTC mode
	TCCR1B |= (1<<WGM12);
	
	//prescale by 1024 -> 15.625 kHz
	TCCR1B |= (1<<CS12) | (1<<CS10);
	
	//Set count limit, 31250*(1/15625) = 2 s
	OCR1A = 31250;
	
	//Enable compare register interrupt
	TIMSK1 |= (1<<OCIE0A);
}

/*Entering to sleep mode if on/off switch is set*/
ISR(PCINT2_vect)
{
	//if pin PD2 is low, set sleep mode
	if (SYSTEM_SLEEP)
	{
		//Disable ADC
		ADCSRA &= ~(1<<ADEN);
		
		//Disabling blocks in PRR
		PRR |= (1<<PRTIM0) | (1<<PRTIM1) | (1<<PRTIM2);
		
		set_sleep_mode(SLEEP_MODE_PWR_DOWN); // choose power down mode
		cli(); // disable interrupts
		sleep_enable(); //setting SE-bit
		sei(); // enable interrupts
		sleep_cpu(); // sleep now
		// wake up point
		sleep_disable(); // clearing SE-bit
		sei();

		//Enable ADC
		ADCSRA |= (1<<ADEN);
		
		//Enable PRR
		PRR &= ~(1<<PRTIM0) & ~(1<<PRTIM1) & ~(PRTIM2);
	}
}

/*Transferring data to virtual terminal every 2 seconds*/
ISR (TIMER1_COMPA_vect)
{
	//Update right values to info
	//scale servo position to range 0-3
	uint8_t scaledServoPosition = 3*((float)servoPosition/255);
	info[7] = (scaledServoPosition + '0');
	
	//scale temperature to range 0-9
	uint8_t scaledTemperature = 9*((float)potentiometerTemperature/255);
	info[23] = (scaledTemperature + '0');
	
	int i;
	for (i=0; info[i]!=0; i++)
	{
		USARTtransmit(info[i]);
	}
	//One empty row
	USARTtransmit(13);
	USARTtransmit(13);
}

/*Receiving data from virtual terminal*/
ISR (USART_RX_vect)
{
	//read data from buffer
	unsigned char dataFromVirtualTerminal = UDR0;
	
	if (dataFromVirtualTerminal == '-')
	{
		if (virtualTerminalsetting > 1)
			virtualTerminalsetting--;
	}
	if (dataFromVirtualTerminal == '+')
	{
		if (virtualTerminalsetting < 255)
			virtualTerminalsetting++;
	}
	
	//reset virtual terminal command
	dataFromVirtualTerminal = ' ';
}

/*Reading analog-to-digital converter result from given channel*/
int readADC(uint8_t channelNumbber)
{
	//set the channel
	//(ADMUX & 0xF0) is the four leftmost bits of ADMUX
	//(channelNumber & 0x0F) is the four rightmost bits of channelNumber
	ADMUX = (ADMUX & 0xF0) + (channelNumbber & 0x0F);
	
	//Star conversion
	ADCSRA |= (1<<ADSC);
	
	//Wait until the conversion is ready by polling
	while (ADCSRA & (1<<ADSC)) ;
	
	//return value
	return ADCH;
}

/*Transmit data to virtual terminal*/
void USARTtransmit(unsigned char data)
{
	//Wait for empty transmit buffer
	while( !(UCSR0A & (1<<UDRE0)) ) ;
	
	//Put data into buffer, sends the data
	UDR0 = data;
}

/*Calculate and set servo position*/
void setServoPosition(void)
{
	potentiometerTemperature = readADC(TEMPERATURE);
	potentiometerServo = readADC(SERVO);
	
	//Calculate pulse to servo motor
	servoPosition = (potentiometerTemperature+potentiometerServo+virtualTerminalsetting)/3;
	
	//Set compare value to servo PWM, scaled to range 62-125
	//so pulse is in range 1 ms - 2 ms.
	OCR0A = (125-62)*((float)servoPosition/255)+62;
	
	//Set compare value to led PWM
	OCR2B = servoPosition;
}

//Software generated interrupt to set sleep mode if needed
void setStartingState(void)
{
	DDRD ^= (1<<DDD2);
	DDRD ^= (1<<DDD2);
}

int main(void)
{
	initTimer1();
	initUSART(MYUBRR);
	initADC();
	initServoPWM();
	initOnOffSwitch();
	setStartingState();
	initLedPWM();
	
	sei();
	
    while (1) 
    {
		setServoPosition();
    }
}

