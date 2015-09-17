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

#define SENSE 1<<PB5
#define HOURT 1<<PB2
#define HOURO 1<<PB3
#define MINST 1<<PB1
#define MINSO 1<<PB0


uint8_t hourTen = 1;
uint8_t hourOne = 2;
uint8_t minTen = 5;
uint8_t minOne = 3;

void init_IO(void){
    //LEDs
    DDRC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4);    //Set control pins as outputs
    PORTC &= ~((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4));    //Set control pins low

    //Input pin
    DDRC &= ~(1<<PC5); 
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
