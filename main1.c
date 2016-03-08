
#include <avr/io.h>
#include <avr/interrupt.h>



// #include "C:\Users\student\Downloads\header files\io.c"
// #include "C:\Users\student\Downloads\header files\io.h"

// #include "C:\Users\IEUser\Downloads\includes\io.c"
// #include "C:\Users\IEUser\Downloads\includes\io.h"

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.

unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks


void TimerOn() {

	// AVR timer/counter controller register TCCR1

	// bit3 = 0: CTC mode (clear timer on compare)

	// bit2bit1bit0=011: pre-scaler /64

	// 00001011: 0x0B

	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s

	// Thus, TCNT1 register will count at 125,000 ticks/s

	TCCR1B = 0x0B;

	// AVR output compare register OCR1A.

	// Timer interrupt will be generated when TCNT1==OCR1A

	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125

	// So when TCNT1 register equals 125,

	// 1 ms has passed. Thus, we compare to 125.

	OCR1A = 125;// AVR timer interrupt mask register

	// bit1: OCIE1A -- enables compare match interrupt

	TIMSK1 = 0x02;

	//Initialize avr counter

	TCNT1=0;

	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	_avr_timer_cntcurr = _avr_timer_M;

	//Enable global interrupts: 0x80: 1000000

	SREG |= 0x80;

}
void TimerOff() {

	// bit3bit1bit0=000: timer off

	TCCR1B = 0x00;

}

void TimerISR() {

	TimerFlag = 1;

}
ISR(TIMER1_COMPA_vect) {

	// CPU automatically calls when TCNT1 == OCR1

	// (every 1 ms per TimerOn settings)

	// Count down to 0 rather than up to TOP (results in a more efficient comparison)

	_avr_timer_cntcurr--;

	if (_avr_timer_cntcurr == 0) {

		// Call the ISR that the user uses

		TimerISR();

		_avr_timer_cntcurr = _avr_timer_M;

	}

}
void TimerSet(unsigned long M) {

	_avr_timer_M = M;

	_avr_timer_cntcurr = _avr_timer_M;

}

void transmit_green(unsigned short data) {
	for (int i = 10; i >= 0; i--) {
		PORTC = 0x08;
		PORTC |= ((data >> i) & 0x01);
		PORTC |= 0x04;
	}
	PORTC |= 0x02;
	PORTC = 0x00;
}

void transmit_yellow(unsigned short data) {
	for (int i = 10; i >= 0; i--) {
		PORTD = 0x08;
		PORTD |= ((data >> i) & 0x01);
		PORTD |= 0x04;
	}
	PORTD |= 0x02;
	PORTD = 0x00;
}

//Transmit data upper 4 bits of PORTC Blue colors
void transmit_red(unsigned short data) {
	for (int i = 7; i >= 0; i--) {
		PORTD = 0x80;
		PORTD |= (((data >> i) & 0x01)<< 4);
		PORTD |= 0x40;
	}
	
	//set RCLK = 1. Rising edge copies data from the “Shift” register to the
	PORTD |= 0x20;
	PORTD = 0x00;
}

void transmit_blue(unsigned short data) {
	for (int i = 7; i >= 0; i--) {
		PORTC = 0x80;
		PORTC |= (((data >> i) & 0x01)<< 4);
		PORTC |= 0x40;
	}
	
	//set RCLK = 1. Rising edge copies data from the “Shift” register to the
	PORTC |= 0x20;
	PORTC = 0x00;
}

unsigned short tmpB;
unsigned char tmpValue;
unsigned short my_short;
unsigned char my_char1;
unsigned char my_char2;


// initialize adc
void ADC_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);

	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}



unsigned short xADC;
unsigned short yADC;

enum IN_States { Start_IN, Wait_IN } IN_State;

uint16_t readadc(uint8_t ch)
{
	ch&=0b00000111;         //ANDing to limit input to 7
	ADMUX = (ADMUX & 0xf8)|ch;  //Clear last 3 bits of ADMUX, OR with ch
	ADCSRA|=(1<<ADSC);        //START CONVERSION
	while((ADCSRA)&(1<<ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
	

	return(ADC);        //RETURN ADC VALUE
}

unsigned char movement;

unsigned short column = 0x04;
unsigned short row = 0x04;

enum States {start, pos } state;



int GetChangeValue( unsigned char loc )
{
	switch( loc )
	{
		case 0x01: return 0;
		case 0x02: return 1;
		case 0x04: return 2;
		case 0x08: return 3;
		case 0x10: return 4;
		case 0x20: return 5;
		case 0x40: return 6;
		case 0x80: return 7;
	}
}

int getRow( unsigned char curValue )
{
	if      ( curValue == 1 )	return 0x01;
	else if ( curValue == 2 )	return 0x02;
	else if ( curValue == 3 )	return 0x04;
	else if ( curValue == 4 )	return 0x08;
	else if ( curValue == 5 )	return 0x10;
	else if ( curValue == 6 )	return 0x20;
	else if ( curValue == 7 )	return 0x40;
	else if ( curValue == 0 )	return 0x80;
}

int getCol( unsigned char curValue )
{
	if		( curValue <= 8 )  return 0x01;
	else if ( curValue <= 16 ) return 0x02;
	else if ( curValue <= 24 ) return 0x04;
	else if ( curValue <= 32 ) return 0x08;
	else if ( curValue <= 40 ) return 0x10;
	else if ( curValue <= 48 ) return 0x20;
	else if ( curValue <= 56 ) return 0x40;
	else if ( curValue <= 64 ) return 0x80;
	
}

unsigned char map[64] = { 1, 2, 3, 4, 5, 6, 7, 8,
						9, 10, 11, 12, 13, 14, 15, 16,
						17, 18, 19, 20, 21, 22, 23, 24,
						25, 26, 27, 28, 29, 30, 31, 32,
						33, 34, 35, 36, 37, 38, 39, 40,
						41, 42, 43, 44, 45, 46, 47, 48,
						49, 50, 51, 52, 53, 54, 55, 56,
						57, 58, 59, 60, 61, 62, 63, 64 };


unsigned char currentPosition;
unsigned char getModulus;
unsigned char tmpPos;
unsigned char rowPos;
unsigned char colPos;

unsigned char hitDetected = 0;

void tick( short xADC, short yADC )
{
	int c = 0;
	switch( state )
	{
		case start:
		state = pos;
		currentPosition = 3;
		break;

		case pos:
		
		/*LCD_ClearScreen( );*/
		
		// 		PORTB = GetChangeValue( row );
		
		// 		playerArray [ GetChangeValue( row ) ] = playerArray[GetChangeValue( row ) ] | column;
		
		if ( hitDetected != 1 )
		{
			//	Center	//////////////////////////////////////////////////////////
			if ( ( xADC >= 540 && xADC <= 570 ) && ( yADC >= 540 && yADC <= 570 ) )
			{
				/*LCD_DisplayString( 1, "Center" );*/
				movement = 'C';

			}
			
			//	Up	////////////////////////////////////////////////////////////////////
			else if ( ( xADC >= 530 && xADC <= 650 ) && ( yADC >= 800 && yADC <= 1100 ) )
			{
				/*LCD_DisplayString( 1, "Up" );*/
				movement = 'U';

				if ( map[ currentPosition ] > 8 )
				{
					currentPosition = currentPosition - 8;
					//playerArray[ GetChangeValue ( row ) ] = 0x00;
					//row = row >> 1;
					
				}

				
			}
			
			//	Down	/////////////////////////////////////////////////////////////
			else if ( ( xADC >= 530 && xADC <= 650 ) && ( yADC >= 0 && yADC <= 200 ) )
			{
				/*LCD_DisplayString( 1, "Down" );*/
				movement = 'D';
				if ( map[ currentPosition ] < 57 )
				{
					currentPosition = currentPosition + 8;
				}

			}
			
			//Left	/////////////////////////////////////////////////////////////
			else if ( ( xADC >= 0 && xADC <= 200 ) && ( yADC >= 400 && yADC <= 600 ) )
			{
				/*LCD_DisplayString( 1, "Left" );*/
				movement = 'L';
				
				if ( ( map[ currentPosition ] % 8 ) != 0  )
				currentPosition = currentPosition + 1;
			}
			
			//	Right	////////////////////////////////////////////////////////////////
			else if ( ( xADC >= 800 && xADC <= 1100 ) && ( yADC >= 400 && yADC <= 600 ) )
			{
				//LCD_DisplayString( 1, "Right" );
				movement = 'R';
				
				if ( ( map[ currentPosition ] % 8 ) != 1 )
				currentPosition = currentPosition - 1;
			}
			
			//	Top Left	///////////////////////////////////////////////////////////
			else if ( ( xADC >= 100 && xADC <= 300 ) && ( yADC >= 600 && yADC <= 900 ) )
			{
				//LCD_DisplayString( 1, "Top Left" );
				
				if ( ( ( map[ currentPosition ] % 8 ) != 0 ) && map[ currentPosition ] > 8 )
				{
					currentPosition = currentPosition - 7;
				}
			}
			
			//	Top Right	///////////////////////////////////////////////////////////
			else if ( ( xADC >= 700 && xADC <= 950 ) && ( yADC >= 600 && yADC <= 900 ) )
			{
				//LCD_DisplayString( 1, "Top Right" );
				
				if ( ( ( map[ currentPosition ] % 8 ) != 1 ) && map[ currentPosition] > 8  )
				{
					currentPosition = currentPosition - 9;
				}
			}
			
			//	Bottom Left	///////////////////////////////////////////////////////////
			else if ( ( xADC >= 100 && xADC <= 300 ) && ( yADC >= 100 && yADC <= 350 ) )
			{
				//LCD_DisplayString( 1, "Bottom Left" );
				
				if ( ( ( map[ currentPosition ] % 8 ) != 0 ) && map[ currentPosition] < 57  )
				{
					currentPosition = currentPosition + 9;
				}
			}
			
			//	Bottom Right	///////////////////////////////////////////////////////
			else if ( ( xADC >= 700 && xADC <= 950 ) && ( yADC >= 100 && yADC <= 350 ) )
			{
				//LCD_DisplayString( 1, "Bottom Right" );
				
				if ( ( ( map[ currentPosition ] % 8 ) != 1 ) && map[ currentPosition] < 57 )
				{
					currentPosition = currentPosition + 7;
				}
			}
			else
			;
			
			getModulus = map[ currentPosition ] % 8;
			rowPos = getRow( getModulus );
			
			tmpPos = map[ currentPosition ];
			colPos = getCol( tmpPos );
			
			
			//transmit_green( ~rowPos );
			//transmit_yellow( colPos );
			
			break;
		}
	}
}

enum ZZ_States{ ZZ_Start, ZZ_Go, ZZ_Freeze } ZZ_state;

unsigned char currentZZ;
unsigned char zzModulus;
unsigned char tmpZZ	;
unsigned char zigZagR;
unsigned char zigZagC;

void ZZ_Tick( )
{
	static unsigned char moveToggle;
	static unsigned char contFlag;
	switch( ZZ_state )
	{
		case ZZ_Start:
		ZZ_state = ZZ_Go;
		
		currentZZ = 5;
		zigZagC = 0x01;
		
		moveToggle = 0;
		contFlag = 1;
		
		break;
		
		case ZZ_Go:
		tmpZZ = map[ currentZZ ] % 8;
		zigZagR = getRow(  tmpZZ );
		
		// 			transmit_blue( ~zigZagR );
		// 			transmit_yellow( zigZagC );
		
		contFlag = 0;
		
		if ( map[ currentZZ ] == 63 || map[ currentZZ ] == 61 )
		{
			currentZZ = 5;
			zigZagC = 0x01;
			contFlag = 1;
		}
		
		if ( contFlag == 0 )
		{
			if ( moveToggle == 0x00 )
			{
				currentZZ = currentZZ + 7;
				zigZagC = zigZagC << 1;
				moveToggle = ~moveToggle;
			}
			
			else
			{
				currentZZ = currentZZ + 9;
				zigZagC = zigZagC << 1;
				moveToggle = ~moveToggle;
				
			}
		}
		
		if ( hitDetected == 1 )
		{
			ZZ_state = ZZ_Freeze;
			break;
		}
		
		// 			transmit_blue( ~zigZagR );
		// 			transmit_yellow( zigZagC );
		break;
		
		case ZZ_Freeze:
		break;
		
		
	}
}

enum LZ_States{ LZ_Start, LZ_Beam, LZ_Freeze } LZ_state;

unsigned char currentLZ;
unsigned char LZModulus;
unsigned char tmpLZ	;
unsigned char LZR;
unsigned char LZC;

void LZ_Tick( )
{
	switch( LZ_state )
	{
		case LZ_Start:
		LZ_state = LZ_Beam;
		
		LZR = 0x80;
		LZC = 0xFF;
		
		break;
		
		case LZ_Beam:
		break;
		
	}
}

enum BL_States{ BL_Start, BL_Shoot, BL_Freeze } BL_state;

unsigned char currentBL;
unsigned char BLModulus;
unsigned char tmpBL	;
unsigned char BLR;
unsigned char BLC;
unsigned char littleDelay;

void BL_Tick( )
{
	switch( BL_state )
	{
		case BL_Start:
		BL_state = BL_Shoot;
		
		BLR = 0x02;
		BLC = 0x01;
		
		tmpBL = 0;
		
		littleDelay = 0;
		
		break;
		
		case BL_Shoot:
			if ( littleDelay >= 10 )
			{
				BLC = BLC << 1;
				tmpBL++;
		
				if ( tmpBL >= 8 )
				{
					BLC = 0x01;
					tmpBL = 0;
				}
			}
			
			littleDelay++;
			
		break;
		
	}
}

enum MD_States{ MD_Start, MD_MatrixDisplay, MD_Pause } MD_state;

void MD_Tick( )
{
	switch( MD_state )
	{
		case MD_Start:
		MD_state = MD_MatrixDisplay;
		hitDetected = 0;
		break;
		
		case MD_MatrixDisplay:
		if ( hitDetected == 1 )
		{
			PORTB = 0x01;
			transmit_red( ~0xFF );
			transmit_yellow( 0xFF );
			MD_state = MD_Pause;
		}
		
		else if ( hitDetected == 0 )
		{
			transmit_blue( ~zigZagR );
			transmit_yellow( zigZagC );
			
			transmit_blue( ~0x00 );
			transmit_yellow( 0x00 );
			
			transmit_green( ~rowPos );
			transmit_yellow( colPos );
			
			transmit_green( ~0x00 );
			transmit_yellow( 0x00 );
			
// 			transmit_red( ~0x80);
// 			transmit_yellow( LZC );
// 						
// 			transmit_red( ~0x00 );
// 			transmit_yellow( 0x00 );

			transmit_blue( ~LZR );
			transmit_yellow( LZC) ;
			
			transmit_blue( ~0x00 );
			transmit_yellow( 0x00 );
			
			transmit_green( ~LZR );
			transmit_yellow( LZC );
			
			transmit_green( ~0x00 );
			transmit_yellow( 0x00 );
						
			transmit_blue( ~0x02 );
			transmit_yellow( BLC );			
			
			transmit_blue( ~0x00 );
			transmit_yellow( 0x00 );			
			
		}
		
		break;
		
		case MD_Pause:
		break;
		
	}
}

enum HD_States{ HD_Start, HD_Scan } HD_state;

void HD_Tick( )
{
	switch( HD_state )
	{
		case HD_Start:
		HD_state = HD_Scan;
		break;
		
		case HD_Scan:
		if ( ( zigZagR == rowPos && zigZagC == colPos ) || ( LZR == rowPos) || ( BLR == rowPos && BLC == colPos ) )
			hitDetected = 1;
		
		break;
	}
}


unsigned char period = 1;

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	unsigned long state_elapsedTime = 100;
	unsigned long zz_elapsedTime = 100;
	unsigned long md_elapsedTime = 1;
	unsigned long hd_elapsedTime = 1;
	unsigned long lz_elapsedTime = 1;
	unsigned long bl_elapsedTime = 100;
	
	unsigned long state_maxTime = state_elapsedTime;
	unsigned long zz_maxTime = zz_elapsedTime;
	unsigned long hd_maxTime = hd_elapsedTime;
	unsigned long lz_maxTime = lz_elapsedTime;
	unsigned long bl_maxTime = bl_elapsedTime;
	
	TimerSet( period );
	TimerOn();
	
	ADC_init( );
	//LCD_init( );
	
	state = start;
	ZZ_state = ZZ_Start;
	MD_state = MD_Start;
	HD_state = HD_Start;
	LZ_state = LZ_Start;
	BL_state = BL_Start;
	

	// 	unsigned char zigZagR= 0x04;
	// 	unsigned char zigZagC = 0x01;
	// 	unsigned char moveToggle = 0;
	//
	transmit_red ( ~0x00 );
	transmit_green( ~0x00 );
	transmit_blue( ~0x00 );
	transmit_yellow( 0x00 );
	
	while(1)
	{
		
		tmpB = ~PINA & 0x03;
		
		xADC = readadc( 0 );	// gets X
		yADC = readadc( 1 );	// gets Y
		
		// 		transmit_blue( ~0x00 );
		// 		transmit_yellow( 0x00 );
		if ( hd_elapsedTime >= hd_maxTime )
		{
			HD_Tick();
			hd_elapsedTime = 0;
		}
		
		if ( md_elapsedTime >= 1 )
		{
			MD_Tick();
			md_elapsedTime = 0;
		}
		

		if ( state_elapsedTime >= state_maxTime )
		{
			tick( xADC, yADC );
			state_elapsedTime = 0;
		}
		
		if ( zz_elapsedTime >= zz_maxTime )
		{
			ZZ_Tick();
			zz_elapsedTime = 0;
		}
		
		if ( lz_elapsedTime >= lz_maxTime )
		{
			LZ_Tick();
			lz_elapsedTime = 0;
		}
		
		if ( bl_elapsedTime >= bl_maxTime )
		{
			BL_Tick();
			bl_elapsedTime = 0;
		}
		
		
		while (!TimerFlag);
		TimerFlag = 0;
		
		
		state_elapsedTime += period;
		zz_elapsedTime += period;
		md_elapsedTime += period;
		hd_elapsedTime += period;
		lz_elapsedTime += period;
		bl_elapsedTime += period;
		
		//LaserTick();
		

		

		// 		while (!TimerFlag);
		// 		TimerFlag = 0;


		

	}
}
