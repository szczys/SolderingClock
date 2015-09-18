/********************************
* Soldering Clock               *
* MIT License               	*
* Copyright 2015 - Mike Szczys  *
* http://jumptuck.com 	    	*
*				*
********************************/

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define SENSE       1<<PB5
#define HOURT       1<<PB2
#define HOURO       1<<PB3
#define MINST       1<<PB1
#define MINSO       1<<PB0

#define BUTSHOW     1<<PD1
#define BUTHOUR     1<<PD0
#define BUTMIN      1<<PD2

volatile uint8_t showTimeFlag = 0;
volatile uint8_t hourTen = 1;
volatile uint8_t hourOne = 2;
volatile uint8_t minTen = 5;
volatile uint8_t minOne = 3;
volatile uint8_t seconds = 0;

void init_IO(void){
    //LEDs
    DDRC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4);    //Set control pins as outputs
    PORTC &= ~((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4));    //Set control pins low

    //Input pin
    DDRC &= ~(1<<PC5);
    
    //Buttons
    DDRD &= ~(BUTSHOW | BUTHOUR | BUTMIN);
    PORTD |= BUTSHOW | BUTHOUR | BUTMIN; //Pullup resistors
}

void init_RTC(void){
    //RTC Example Code from Atmel (altered for m168)
    //http://www.atmel.com/Images/Atmel-1259-Real-Time-Clock-RTC-Using-the-Asynchronous-Timer_AP-Note_AVR134.zip
	TIMSK2 &= ~((1<<TOIE2)|(1<<OCIE2B));						//Make sure all TC0 interrupts are disabled
	ASSR |= (1<<AS2);										//set Timer/counter0 to be asynchronous from the CPU clock
															//with a second external clock (32,768kHz)driving it.								
	TCNT2 =0;												//Reset timer
	TCCR2B =(1<<CS20)|(1<<CS22);								//Prescale the timer to be clock source/128 to make it
															//exactly 1 second for every overflow to occur
	while (ASSR & ((1<<TCN2UB)|(1<<OCR2BUB)|(1<<TCR2BUB)))	//Wait until TC0 is updated
	{}
	TIMSK2 |= (1<<TOIE2);									//Set 8-bit Timer/Counter0 Overflow Interrupt Enable
	sei();													//Set the Global Interrupt Enable Bit
}

void twiddle(void){
    PORTC |= 1<<PC4;
    _delay_ms(10);
    PORTC &= ~(1<<PC4);
    _delay_ms(10);
}

void readInput(void){
    if (PINC & (1<<PC5)) {
        PORTC |= 1<<PC0;
        _delay_ms(1000);
    }
    else { PORTC &= ~(1<<PC0); }
}

void findZero(void) {
    while(1) {
        if (PINC & SENSE) { break; }
        else { twiddle(); }
    }
    for (uint8_t i=0; i<7; i++) { twiddle(); }
}

void showNumber(uint8_t num) {
    findZero();
    
    for (uint8_t i=0; i<num; i++) { twiddle(); }
}

void showTime(void) {
    //Hours Tens
    if (hourTen > 0) {
        showNumber(hourTen);
        PORTC |= HOURT;
        _delay_ms(1000);
        PORTC &= ~HOURT;
    }
    
    //Hours Ones
    showNumber(hourOne);
    PORTC |= HOURO;
    _delay_ms(1000);
    PORTC &= ~HOURO;
    
    //Minutes Tens
    showNumber(minTen);
    PORTC |= MINST;
    _delay_ms(1000);
    PORTC &= ~MINST;

    //Minutes Ones
    showNumber(minOne);
    PORTC |= MINSO;
    _delay_ms(1000);
    PORTC &= ~MINSO;
}

int main(void)
{
    init_IO();
    init_RTC();
    while(1)
    {
        if ((PIND & BUTSHOW) != BUTSHOW) { showTime(); }
        if (showTimeFlag) {
            showTimeFlag = 0;
            showTime();
        }
    }


    while(1)
    {
	//    twiddle();
	//readInput();
        findZero();
        findZero();
        findZero();
        _delay_ms(1000);
        
        showTime();

/*
	PORTC |= 1<<PC4;
	_delay_ms(200);
	PORTC &= ~(1<<PC4);
	_delay_ms(200);
*/
    }
}

ISR(TIMER2_OVF_vect) {
    //PINC |= MINST;      //Toggle LED at 1Hz
    if (++seconds == 60) {
        seconds = 0;
        showTimeFlag = 1;
        if (++minOne == 10) {
            minOne = 0;
            if (++minTen == 6) {
                minTen = 0;
                ++hourOne;
                if (hourTen && (hourOne == 3)) {
                    hourTen = 0;
                    hourOne = 1;
                }
                else if (hourOne == 10) {
                    hourOne = 0;
                    ++hourTen;
                }
            }
        }
    }
}
