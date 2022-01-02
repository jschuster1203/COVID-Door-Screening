/*
 * oledTest.c
 *
 * Created: 4/18/2021 8:15:48 PM
 * Author : Jack
 */ 
//#define F_CPU 16000000
#include <avr/io.h>

#include "Display/lcd.h"
#include <util/delay.h>
#include <avr/interrupt.h>

#define startbutton 0 //start button is PD0
#define nobutton 1 //no button for questions PD1 (left button)
#define yesbutton 2 //yes button for questions PD2 (right button)
#define greenOccpin 3 //green occupancy led on PD3
#define yellowOccin 4 //yellow occ led on PD4
#define redOccpin 5 //red occ led on PD5
#define greenCheck 6 //green led on PD6 for blinking when someone enters room
#define redCheck 7 //red led on PD7 for blinking when someone fails questions
#define TRIGGER 1 //PB1 for distance sensor
#define motor 2 //PB2 for motor gpio


float maxOcc= 5;//maximum occupancy value of the room
float occ = 4; //current occupancy
float percentage = 0;//percentage of room occupancy

int loopbreak = 0;
int counter = 0; //counter for waiting 60 seconds for question response
int distanceCount= 0;//counter used to track time in front of sensor

int screeningQuestions(void);//screening question process
void led_reset(void);//function to clear led screen and set goto to 0,0
void blinkGreen(void);//function to blink green LED
void blinkRed(void);//function to blink red LED
void getDistance(void);//function to get distance from door


int main(void)
{
	lcd_init(LCD_DISP_ON);//initialize OLED
	led_reset();
    
	DDRD |= (1<<greenOccpin) | (1<<yellowOccin) | (1<<redOccpin) | (1<<greenCheck) | (1<<redCheck);//set occupancy LEDs & blinking LED to outputs and buttons to inputs 
	PORTD |= (1<<startbutton) | (1<<nobutton) | (1<<yesbutton);//PD0, PD1, PD2 input pullup, if PIND0-2 is low, switched closed
	DDRB |= (1<<motor);//set motor pin to output
	PORTB &= ~(1<<motor);//make sure motor is off at start
	
	
    while (1) 
    {
		percentage = occ /maxOcc;//calculate current percentage
		
		if(percentage <= 0.4){//if occupancy is below 40%, turn on green LED and turn off yellow and red
			PORTD |= (1<<greenOccpin);
			PORTD &= ~(1<<yellowOccin) & ~(1<<redOccpin);
		}
		
		if((percentage >0.4) && (percentage <0.8)){ //if occupancy is between 41% and 79%, yellow LED on, other 2 off
			PORTD |= (1<<yellowOccin);
			PORTD &= ~(1<<greenOccpin) & ~(1<<redOccpin);
		}
		
		if(percentage >= 0.8){
			PORTD |= (1<<redOccpin);
			PORTD &= ~(1<<greenOccpin) & ~(1<<yellowOccin);
		}
		
		while(((PIND & (1<<startbutton))!=0) || (percentage >= 0.8)){//keep looping until start button is pressed
			getDistance();//distance check
			if(percentage < 0.8){
				lcd_gotoxy(0,0);
				lcd_puts("Welcome to the COVID\r\nScreening. Press the \r\nfar left button to\r\nbegin");//show message on OLED screen
			}
			if(percentage >=0.8){
				lcd_gotoxy(0,0);
				lcd_puts("Room is currently\r\nfull, please wait for\r\nlight to be yellow\r\nto start.");
			}
				
		}
		led_reset();//clear led and reset screen
		
		while(1){
			loopbreak = screeningQuestions();
			if(loopbreak == 1){ //answered yes (show symptoms)
				getDistance();//distance check
				led_reset();
				lcd_puts("Answer shows symptoms\r\ncan not allow room\r\nentrance.");
				blinkRed();//blink red led
				_delay_ms(5000);//delay for 5 seconds
				led_reset();
				break;
			}
			if(loopbreak == 2){//timed out
				getDistance();//distance check
				led_reset();
				lcd_puts("Question timed out\r\nPlease start again.");
				blinkRed();//blink red led
				_delay_ms(5000);
				led_reset();
				break;
			}
			if(loopbreak == 0){//passed questions
				getDistance();//distance check
				led_reset();
				lcd_puts("No symptoms reported\r\nPlease enter room.");
				blinkGreen();
				_delay_ms(2000);
				PORTB |= (1<<motor);//turn on motor
				_delay_ms(500);
				PORTB &= ~(1<<motor); //turn off motor
				occ++;//increase occupancy value
				
				led_reset();
				
				break;
			}
			
		}
		
    }
}


int screeningQuestions(void){//return 1 if symptoms are found, 0 if none
	//with 1024 clock prescalar, overflow every 15625, get it so that interrupt called once a second and increment a value
	counter = 0;//used to check how many seconds passed
	TCNT1 = 65536 - 15625;//should overflow once a second
	TCCR1A = 0x00;//set timer 1 normal mode
	TCCR1B = 5;//normal mode, 1024 clock prescaler
	TIMSK1 = (1 <<TOIE1);//enable timer 1 interrupt
	sei();//enable global interrupt 
	led_reset();
	lcd_puts("Have you experienced\r\nany of these\r\nsymptoms in the\r\npast 48 hours");
	_delay_ms(3000);
	led_reset();
	lcd_puts("Have you had fever\r\nchills or a cough?\r\nNo=Left Yes=Right");
	while(counter <=65){
		if((PIND & (1<<yesbutton))==0){
			led_reset();
			return 1;//return 1 saying that COVID symptom found
		}
		if((PIND & (1<<nobutton))==0){
			_delay_ms(500);
			break;//break out of while loop and keep going
		}
		if(counter >60){
			led_reset();
			return 2;
		}	
	}
	getDistance();//distance check
	
	led_reset();
	lcd_puts("Shortness of breath,\r\ndifficulty breathing\r\nor fatigue?\r\nNo=left, Yes=right");
	counter = 0;//reset counter value
	while(counter <=65){
		if((PIND & (1<<yesbutton))==0){
			led_reset();
			return 1;//return 1 saying that COVID symptom found
		}
		if((PIND & (1<<nobutton))==0){
			_delay_ms(500);//small delay to prevent answering multiple questions
			break;//break out of while loop and keep going
		}
		if(counter >60){
			led_reset();
			return 2;
		}
		
	}
	getDistance();//distance check
	
	led_reset();
	lcd_puts("Muscle,body or\r\nheadaches?\r\nNo=Left, Yes=Right");
	counter = 0;//reset counter value
	while(counter <=65){
		if((PIND & (1<<yesbutton))==0){
			led_reset();
			return 1;//return 1 saying that COVID symptom found
		}
		if((PIND & (1<<nobutton))==0){
			_delay_ms(500);//small delay to prevent answering multiple questions
			break;//break out of while loop and keep going
		}
		if(counter >60){
			led_reset();
			return 2;
		}
		
	}
	getDistance();//distance check
	
	led_reset();
	lcd_puts("New loss of\r\ntaste or smell?\r\nNo=Left, Yes=Right");
	counter = 0;//reset counter value
	while(counter <=65){
		if((PIND & (1<<yesbutton))==0){
			led_reset();
			return 1;//return 1 saying that COVID symptom found
		}
		if((PIND & (1<<nobutton))==0){
			_delay_ms(500);//small delay to prevent answering multiple questions
			break;//break out of while loop and keep going
		}
		if(counter >60){
			led_reset();
			return 2;
		}
		
	}
	TIMSK1 = (0 <<TOIE1);//disable timer 1 interrupt
	return 0;
}

void led_reset(void){
	lcd_clrscr(); //clear OLED screen
	lcd_gotoxy(0,0); //set text at 0,0
}

void blinkGreen(void){
	for(int i = 0; i<7;i++){
		PORTD |= (1<<greenCheck);//green led on
		_delay_ms(100);//wait
		PORTD &= ~(1<<greenCheck);//green led off
		_delay_ms(100);//delay
	}
}

void blinkRed(void){
	for(int i = 0; i<7;i++){
		PORTD |= (1<<redCheck);//green led on
		_delay_ms(100);//wait
		PORTD &= ~(1<<redCheck);//green led off
		_delay_ms(100);//delay
	}	
}

void getDistance(void){
	unsigned int timeToRisingEdge, timeToFallingEdge, pulseWidth;//variables for the duration of sound wave travel
	int distance; //variable for the distance measurement
	DDRB |= (1<<TRIGGER);//set TRIGGER (PB1) to output
	TCCR1A = 0;//timer 1 on normal
	
	PORTB |= (1<<TRIGGER);//provide 10.0 us pulse to trigger pin
	_delay_us(10.);
	PORTB &= ~(1<<TRIGGER);
	TCNT1 = 0x00;
	
	TCCR1B = 0x45; //start counting till rising edge with prescaler 1024
	while((TIFR1 & (1<<ICF1))==0);//wait for rising edge on ICP1/PB0
	timeToRisingEdge = ICR1;//save rising edge delay
	TIFR1 = (1<<ICF1);//clear ICF1 flag so this can be performed again
	
	TCCR1B = 0x05; //start counting till falling edge, prescaler of 1024
	while ((TIFR1 & (1 <<ICF1)) == 0); //wait for falling edge of ICP1/PB0
	timeToFallingEdge = ICR1;
	TIFR1 = (1 <<ICF1); //clear ICF1 flag to be done again
	
	pulseWidth = timeToFallingEdge - timeToRisingEdge; //time delay between edges
	distance = pulseWidth * 1.098;//one way distance to target in cm
	
	if(distance <47){//person is within 1.5 feet of the sensor
		//with TIMER0,  flags interrupt ~61/sec. if we want to see if this last for 2 seconds, we set a variable to increment each time interrupt is called and wait for it to get greater than 122
		TCCR0A = 0x00;//normal mode
		TCCR0B = 5;//normal mode, 1024 prescaler
		TIMSK0 = (1<<TOIE0);//enable timer 0 interrupt
		sei();//enable global interrupt
		distanceCount = 0;
		while((distance <47) && distanceCount<150){
			
			PORTB |= (1<<TRIGGER);//provide 10.0 us pulse to trigger pin
			_delay_us(10.);
			PORTB &= ~(1<<TRIGGER);
			TCNT1 = 0x00;
			
			TCCR1B = 0x45; //start counting till rising edge with prescaler 1024
			while((TIFR1 & (1<<ICF1))==0);//wait for rising edge on ICP1/PB0
			timeToRisingEdge = ICR1;//save rising edge delay
			TIFR1 = (1<<ICF1);//clear ICF1 flag so this can be performed again
			
			TCCR1B = 0x05; //start counting till falling edge, prescaler of 1024
			while ((TIFR1 & (1 <<ICF1)) == 0); //wait for falling edge of ICP1/PB0
			timeToFallingEdge = ICR1;
			TIFR1 = (1 <<ICF1); //clear ICF1 flag to be done again
			
			pulseWidth = timeToFallingEdge - timeToRisingEdge; //time delay between edges
			distance = pulseWidth * 1.098;//one way distance to target in cm
			
			if(distanceCount>122){
				led_reset();
				lcd_puts("Person exiting room");
				occ = occ-1;
				_delay_ms(2000);
				PORTB |= (1<<motor);//turn on motor
				_delay_ms(500);
				PORTB &= ~(1<<motor); //turn off motor
				_delay_ms(500);
				led_reset();
				break;
				}
			
		}
		percentage = occ / maxOcc;//make sure occupancy is up to date
		
		if(percentage <= 0.4){//if occupancy is below 40%, turn on green LED and turn off yellow and red
			PORTD |= (1<<greenOccpin);
			PORTD &= ~(1<<yellowOccin) & ~(1<<redOccpin);
		}
		
		if((percentage >0.4) && (percentage <0.8)){ //if occupancy is between 41% and 79%, yellow LED on, other 2 off
			PORTD |= (1<<yellowOccin);
			PORTD &= ~(1<<greenOccpin) & ~(1<<redOccpin);
		}
		
		if(percentage >= 0.8){
			PORTD |= (1<<redOccpin);
			PORTD &= ~(1<<greenOccpin) & ~(1<<yellowOccin);
		}
		
	}
}


ISR(TIMER1_OVF_vect){
	counter++; //increments counter every overflow, so once a second
	TCNT1 = 65536 - 15625;
}

ISR(TIMER0_OVF_vect){
	distanceCount++;
	
}