/*************************************
FileName:      Sistema_CIP_Tanque.cpp
Dependencies:
Processor:     ATmega328P
Hardware:      Arduino UNO
Compiler:
Company:       INGER
Created:	   16/02/2015 14:04:50
Author:		   Bruno
**************************************
File Description:

Change History:
Rev   Date         Description
---   ----------   ---------------
1.0   16/02/2015   Initial release

*************************************/

#define F_CPU	16000000
#define ARDUINO	106
#include "Arduino.h"
#include "LiquidCrystal.h"


// Seleccione los pines utilizados en el LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// Prototipos de funciones
int read_LCD_buttons();		// Leer botones




void setup()
{
	lcd.begin(16,2);              // start the library
	lcd.setCursor(0,0);
	lcd.print("Salida Entrada  "); // print a simple message

	pinMode(A1,OUTPUT);
	pinMode(A2,INPUT);
}

void loop()
{
	lcd.setCursor(10,1);            // move to the begining of the second line
	lcd_key = read_LCD_buttons();  // read the buttons

	switch (lcd_key)               // depending on which button was pushed, we perform an action
	{
		case btnRIGHT:
		{
			lcd.print("RIGHT ");
			break;
		}
		case btnLEFT:
		{
			lcd.print("LEFT   ");
			break;
		}
		case btnUP:
		{
			lcd.print("UP    ");
			break;
		}
		case btnDOWN:
		{
			lcd.print("DOWN  ");
			break;
		}
		case btnSELECT:
		{
			lcd.print("SELECT");
			break;
		}
		case btnNONE:
		{
			lcd.print("NONE  ");
			break;
		}
	}
	//lcd.print(millis()/1000);      // display seconds elapsed since power-up
	digitalWrite(A1, HIGH);   // turn the LED on (HIGH is the voltage level)
	lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
	lcd.print("HIGH");
	delay(500);              // wait for a second
	digitalWrite(A1, LOW);    // turn the LED off by making the voltage LOW
	lcd.setCursor(0,1);
	lcd.print("LOW ");
	delay(500);              // wait for a second
	lcd.setCursor(7,1);            // move cursor to second line "1" and 0 spaces over
	lcd.print(digitalRead(A2));
	
}

// Read the buttons
int read_LCD_buttons()
{
	adc_key_in = analogRead(0);     // read the value from the sensor
									// my buttons when read are centered at these valies: 0, 144, 329, 504, 741
									// we add approx 50 to those values and check to see if we are close
	if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
	// For V1.1 us this threshold
	/*
	if (adc_key_in < 50)   return btnRIGHT;
	if (adc_key_in < 250)  return btnUP;
	if (adc_key_in < 450)  return btnDOWN;
	if (adc_key_in < 650)  return btnLEFT;
	if (adc_key_in < 850)  return btnSELECT;
	*/
	// For V1.0 comment the other threshold and use the one below:

	if (adc_key_in < 50)   return btnRIGHT;
	if (adc_key_in < 195)  return btnUP;
	if (adc_key_in < 380)  return btnDOWN;
	if (adc_key_in < 555)  return btnLEFT;
	if (adc_key_in < 790)  return btnSELECT;



	return btnNONE;  // when all others fail, return this...
}


