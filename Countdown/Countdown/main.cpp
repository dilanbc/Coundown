/*
 * Countdown.cpp
 *
 * Created: 5/25/2020 12:57:15 PM
 * Author : Dilan
 */ 
#define  F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>


#include "LCDI2C.h"
//#include "USART_at8.h"

/*Functions*/
void SetSecond();
void SetMinute();
void SetHour();
void SetDate();
void lcdClock();

/*Variables*/
uint8_t interruptChannel=3;
uint64_t i=0;

uint8_t tSecond;  /*EEPROM 0*/
uint8_t tMinute;  /*EEPROM 1*/	
uint8_t tHour;  /*EEPROM 2*/
uint16_t tdate;  /*EEPROM 3,4*/
uint16_t dateCount;
void InitADC()
{
	// Select Vref=AVcc
	ADMUX |= (1<<REFS0);
	//set prescaller to 128 and enable ADC
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN)|(1<<ADIE);
	sei();
		
}

uint16_t ReadADC(uint8_t ADCchannel)
{	ADCSRA&=~(1<<ADIE);
	
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADC;
}

ISR(ADC_vect){
	
	if (ADC<700)
	{ADCSRA&=~(1<<ADIE);
		TIMSK&=~(1<<TOIE1);
		//USART_TxNumber(ADC);
		eeprom_write_byte((uint8_t*)0,tSecond); 
		eeprom_write_byte((uint8_t*)1,tMinute);
		eeprom_write_byte((uint8_t*)2,tHour);
		eeprom_write_word((uint16_t*)3,tdate);
		//USART_TxStringln(" \r\nPowerDown");
		
	}
	ADCSRA |= (1<<ADSC);
	
}
void adcIntruptRefresh(){
	ADMUX = (ADMUX & 0xF0) | (interruptChannel & 0x0F);
	ADCSRA|=(1<<ADIE);
	ADCSRA |= (1<< ADSC);
}

ISR(TIMER1_OVF_vect)
{	TCNT1=34285;
	//LcdBacklight(1);
	lcdClock();
}



void lcdClock(){               //Update Time with Inturrept
	tSecond++;
	if (tSecond>59)		//Second Control
	{tSecond=0;       
	tMinute++;
	if (tMinute>59)		//Minute Control
	{tMinute=0;     
		tHour++;
		if (tHour>23)	//Hour Control
		{tHour=0;
			tdate++;
			if (tdate>999) //Date Control
			{tdate=0;
			}
		}//Hour
	}//Minute
	}//Second
	
}

void displayInit(){            //Initialize  the display clock
	LcdSetCursor(0,2,"D");
	dateCount=eeprom_read_word((uint16_t*)5);
	char sDate[3];
	itoa(dateCount,sDate,10);
	
	if (dateCount<10)
	{LcdSetCursor(0,3,"00");
		LcdSetCursor(0,5,sDate);
		
	}
	if ((9<dateCount)&&(dateCount<100))
	{
		LcdSetCursor(0,3,"0");
		LcdSetCursor(0,4,sDate);
	}
	if ((99<dateCount)&&(dateCount<1000))
	{LcdSetCursor(0,3,sDate);
	}
	
	LcdSetCursor(0,7,"H00:00:00");
	
	
	
	LcdSetCursor(1,2,"D");
	tdate=eeprom_read_word((uint16_t*)3);
	SetDate();
	
	
	LcdSetCursor(1,7,"H");
	tHour=eeprom_read_byte((uint8_t*)2);
	SetHour();
		
	LcdSetCursor(1,10,":");
	tMinute=eeprom_read_byte((uint8_t*)1);
	SetMinute();
	
	tSecond=eeprom_read_byte((uint8_t*)0);
	LcdSetCursor(1,13,":");
	SetSecond();
}
void displayClock(){      //Connected with Main loop
	
	if (tSecond==0)
	{
		if (tMinute==0);
		{
			if (tHour==0)
			{ SetDate();
			}
			SetHour();
		}
		SetMinute();
	}
	SetSecond();
}

void SetSecond(){
	char sSecond[2];
	itoa(tSecond,sSecond,10);
	if (tSecond<10)
	{	sSecond[1]=sSecond[0];
		sSecond[0]='0';
	}
	LcdSetCursor(1,14,sSecond);	
}
void SetMinute(){
	char sMinute[2];
	itoa(tMinute,sMinute,10);
	if (tMinute<10)
	{	sMinute[1]=sMinute[0];
		sMinute[0]='0';
	}
	LcdSetCursor(1,11,sMinute);
}
void SetHour(){
	char sHour[2];
	itoa(tHour,sHour,10);
	if (tHour<10)
	{sHour[1]=sHour[0];
		sHour[0]='0';
	}
	LcdSetCursor(1,8,sHour);
}

void SetDate(){
	char sDate[3];
	itoa(tdate,sDate,10);
	
	if (tdate<10)
	{LcdSetCursor(1,3,"00");
	LcdSetCursor(1,5,sDate);
		
	}
	if ((9<tdate)&&(tdate<100))
	{
		LcdSetCursor(1,3,"0");
		LcdSetCursor(1,4,sDate);
	}
	if ((99<tdate)&&(tdate<1000))
	{LcdSetCursor(1,3,sDate);
	}
	
	
	}

void enterDate(){
	uint16_t datetemp;
	ADCSRA&=~(1<<ADIE);
	LcdCommand(LCD_RETURNHOME);
	LcdCommand(LCD_CLEARDISPLAY);
	LcdSetCursor(0,1,"Date");
	do 
	{
	} while ((PINC&0b10)>>1);  //wait release
	_delay_ms(100);
	LcdSetCursor(0,8,"000");
	LcdSetCursor(1,10,"");
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);
	
	do{
		char adcRead0 [1];
		datetemp=ReadADC(0)/102.4;
		LcdSetCursor(0,8,itoa(datetemp,adcRead0,10));
		LcdSetCursor(1,8,"");
		_delay_ms(100);
	}while(((PINC&0b10)>>1)==0);
	dateCount=datetemp*100;
	do
	{
	} while ((PINC&0b10)>>1);  //wait release
	_delay_ms(250);
	
	do{
		char adcRead0 [1];
		datetemp=ReadADC(0)/102.4;
		LcdSetCursor(0,9,itoa(datetemp,adcRead0,10));
		LcdSetCursor(1,9,"");
		_delay_ms(100);
	}while(((PINC&0b10)>>1)==0);
	dateCount+=datetemp*10;
	do
	{
	} while ((PINC&0b10)>>1);  //wait release
	_delay_ms(250);
	do{
		char adcRead0 [1];
		datetemp=ReadADC(0)/102.4;
		LcdSetCursor(0,10,itoa(datetemp,adcRead0,10));
		LcdSetCursor(1,10,"");
		_delay_ms(100);
	}while(((PINC&0b10)>>1)==0);
	dateCount+=datetemp;
	do
	{
	} while ((PINC&0b10)>>1);  //wait release
	_delay_ms(250);
	eeprom_write_byte((uint8_t*)0,0);
	eeprom_write_byte((uint8_t*)1,0);
	eeprom_write_byte((uint8_t*)2,0);
	eeprom_write_word((uint16_t*)3,0);
	eeprom_write_word((uint16_t*)5,dateCount);	
	LcdCommand(LCD_DISPLAYCONTROL|(LCD_DISPLAYON&~LCD_BLINKON));
	adcIntruptRefresh();
}
bool checkButton(){
	eeprom_write_byte((uint8_t*)0,tSecond);
	eeprom_write_byte((uint8_t*)1,tMinute);
	eeprom_write_byte((uint8_t*)2,tHour);
	eeprom_write_word((uint16_t*)3,tdate);
	uint16_t i=0;
	bool state1=0;
	bool state2=0;
	do 
	{_delay_ms(1);
		i++;
		if (i>3000)             // Set to 3000
		{ state1=1;
			break;
		}
	} while ((PINC&0b10)>>1); //End of State 1
	
	
	if (state1==1)
	{
		LcdCommand(LCD_RETURNHOME);
		LcdCommand(LCD_CLEARDISPLAY);
		LcdSetCursor(0,0,"Memory Clearing");
		i=0;
		do
		{_delay_ms(250);                         //Set to 250
			
			LcdSetCursor(1,i,">");
			i++;
			if (i>15)
			{state2=1;
				break;
				
			}
		} while ((PINC&0b10)>>1);
		
	}
	
	
	if (state2==1)
	{enterDate();
	}
	LcdCommand(LCD_RETURNHOME);
	LcdCommand(LCD_CLEARDISPLAY);
	displayInit();
	return state2;
}
void full(){
	TIMSK&=~(1<<TOIE1);
	while(1){
		LcdSetCursor(0,0,"");
		LcdBacklight(0);
		_delay_ms(1000);
		LcdSetCursor(0,0,"");
		LcdBacklight(1);
		_delay_ms(1000);
		if ((PINC&0b10)>>1){
			
			if (checkButton())
			{break;
			}
	}
}
}

int main(void)
{	/*Install functions*/
	InitADC();
	while(ReadADC(3)<700){}
	adcIntruptRefresh();
	//USART_Init(9600);
	LcdInit(0x27);
	
	
	//USART_TxStringln("Started");
	LcdSetCursor(0,4,"TECHART");
	_delay_ms(1000);
	LcdCommand(LCD_RETURNHOME);
	LcdCommand(LCD_CLEARDISPLAY);
		
	displayInit();
	_delay_ms(100);
	/*One second Pulse*/
	TIMSK|=(1<<TOIE1);  /*Interrupt Enable*/
	TCCR1B |= (1<<CS12);//(1<<CS12); /*Set prescaler 256 and start*/
	TCNT1=34285;
	
	DDRC&=~(1<<1);
	DDRB&=~(1<<PB1);
	//////////////////////////////////////////////////////////////////
	while (1)
	{	
	displayClock();	
	if ((PINC&0b10)>>1){checkButton();}    //Push Button check
	
	/*Check AC Voltage .............	*/
	uint8_t j;
	uint16_t k=0;
	for( j=0;j<16;j++){
	if ((PINB&0b10)>>1){
	k|=1<<j	;
	}
	else{
	k&=~(1<<j);	
	}
	_delay_ms(20);
	}
	if (k!=0)
	{TIMSK|=(1<<TOIE1);
		LcdSetCursor(1,0,">");
	} 
	else
	{TIMSK&=~(1<<TOIE1);
		LcdSetCursor(1,0," ");
	}
	/*End of Check AC Voltage .............*/
	
	
	/*Timer Overflow Check*/
	if (tdate>=dateCount){
		full();
	}
	
	
	}// end of while(1)
}

