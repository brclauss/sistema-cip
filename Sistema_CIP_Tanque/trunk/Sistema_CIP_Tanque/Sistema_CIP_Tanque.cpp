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
1.0   16/02/2015   Initial release.
1.1   16/02/2015   Fuciones de hardware, inicializaciones, esquema general.
1.2   17/02/2015   Funci�n temporizador.
1.3   18/03/2015   Comienzo Grafcet.
1.4   23/03/2015   Correcci�n funci�n temporizador, implementaci�n de etapas basicas.
1.5   24/03/2015   Completaron etapas y alarmas.
1.6   29/03/2015   Manejo de teclado, menus, guardar parametros en EEPROM.
1.7   03/04/2015   Mensaje de fin de ciclo.

*************************************/

#define F_CPU	16000000
#define ARDUINO	106

// Includes
#include "Arduino.h"
#include "LiquidCrystal.h"
#include "EEPROM.h"

// Constantes y defines
// Entradas digitales
#define I0 A1
#define I1 A2
#define I2 A3
#define I3 A4
#define I4 A5
#define I5 2

// Salidas digitales
const int Q_Agua_Fria = 3;
const int Q_Bomba_Agitador = 11;
const int Q_Desague = 12;
const int Q_Agua_Caliente = 13;

// Etapas del proceso
const int En_Espera = 0;
const int Descarga_Manual =	11;
const int Carga_Agua_Fria_1 = 1;
const int Enjuague_1 = 2;
const int Descarga_Agua_1 = 3;
const int Carga_Agua_Caliente = 4;
const int Lavado = 5;
const int Descarga_Agua_2 = 6;
const int Carga_Agua_Fria_2 = 7;
const int Enjuague_2 = 8;
const int Descarga_Agua_3 = 9;
const int Alarma_Bomba_Agit = 12;
const int Alarma_Llave_T = 13;
const int Final_Ciclo = 14;

// Timers en ms
unsigned long T_Enjuague;
unsigned long T_Descarga;
unsigned long T_Lavado;
const unsigned long T_Rebote = 150;
unsigned char T_Enjuague_s;
unsigned char T_Descarga_s;
unsigned char T_Lavado_s;

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
int aux_key_1 = 0;	// Variable auxiliar para rebote de tecla
int aux_key_2 = 0;	// Variable auxiliar para rebote de tecla

// Variables de programa
unsigned char UC_Etapa = 0;
byte I_Inicio_Parada, I_Nivel, I_Bomba_Agitador, I_Desague_Manual, I_Llave_Tanque, I_Lav_Auto;
boolean M_Agua_Fria, M_Bomba_Agitador, M_Desague, M_Agua_Caliente;
byte M_En_Ciclo = 0;
byte M_En_Alarma = 0;
byte M_En_Espera = 10;
byte M_En_Espera_2 = 100;
byte M_Activar_Timer = 0;
int M_Activar_Timer_2 = 0;

// Variables para Timer
boolean M_Timer_1 = 0;
boolean M_Timer_2 = 0;
boolean M_Timer_3 = 0;
boolean M_Timer_4 = 0;
unsigned long M_Time_1 = 0;
unsigned long M_Time_2 = 0;
unsigned long M_Time_3 = 0;
unsigned long M_Time_4 = 0;
unsigned long TIMER_00 = 0;  // Variable to hold elapsed time for Timer 0
unsigned long TIMER_01 = 0;  // Variable to hold elapsed time for Timer 1
unsigned long TIMER_02 = 0;  // Variable to hold elapsed time for Timer 2
unsigned long TIMER_03 = 0;  // Variable to hold elapsed time for Timer 3
unsigned long TIMER_04 = 0;  // Variable to hold elapsed time for Timer 4

// Variables para menu
byte Modo_Menu = 0;
byte Pantalla = 0;
byte Btn_Select = 0;

// Prototipos de funciones
void ini_in_out(void);		// Inicializa las entradas y salidas
void leer_in(void);			// Lee y almacena todas las entradas digitales
void escribir_outs(void);	// Escribe todas las salidas digitales
void apagar_outs(void);		// Apaga todas las salidas
int read_LCD_buttons();		// Leer botones
unsigned long timerPulse(boolean &timeractive, unsigned long &timeactual, unsigned long &timerState, unsigned long timerPeriod);	// Temporizador
void vigia(void);
int Boton_Con_Rebote(void);


void setup()
{
	lcd.begin(16,2);              // Start the library
	lcd.clear();
	lcd.print("LCD listo...    ");
	delay(1000);
	ini_in_out();
	lcd.clear();
	lcd.print("In/Out listo... ");
	delay(1000);
	T_Enjuague_s = EEPROM.read(0);
	T_Lavado_s = EEPROM.read(1);
	T_Descarga_s = EEPROM.read(2);
	T_Descarga = T_Descarga_s * 60000;
	T_Enjuague = T_Enjuague_s * 60000;
	T_Lavado = T_Lavado_s * 60000;
	lcd.clear();
	lcd.print("EST LA FELICIDAD");
	lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
	lcd.print("   POR  INGER   ");
	delay(3000);
	lcd.clear();
}

void loop()
{	
	// Leer entradas
	leer_in();							// Lee todas las entradas digitales y almacena su valor
	lcd_key = Boton_Con_Rebote();		// read the buttons
	// Cambios de etapa
	// Etapa 1
	if (UC_Etapa == 0 && I_Llave_Tanque == 1 && I_Inicio_Parada == 1)
	{
		UC_Etapa = 1;
		M_En_Ciclo = 1;
	}
	// Etapa 2
	if (UC_Etapa == 1 && I_Nivel == 1)
	{
		UC_Etapa = 2;
	}
	// Etapa 3
	if (UC_Etapa == 2 && M_Timer_1 == 1)
	{
		UC_Etapa = 3;
	}
	// Etapa 4
	if (UC_Etapa == 3 && M_Timer_2 == 1)
	{
		UC_Etapa = 4;
	}
	// Etapa 5
	if (UC_Etapa == 4 && I_Nivel == 1)
	{
		UC_Etapa = 5;
	}
	// Etapa 6
	if (UC_Etapa == 5 && M_Timer_3 == 1)
	{
		UC_Etapa = 6;
	}
	// Etapa 7
	if (UC_Etapa == 6 && M_Timer_2 == 1)
	{
		UC_Etapa = 7;
	}
	// Etapa 8
	if (UC_Etapa == 7 && I_Nivel == 1)
	{
		UC_Etapa = 8;
	}
	// Etapa 9
	if (UC_Etapa == 8 && M_Timer_1 == 1)
	{
		UC_Etapa = 9;
	}
	// Final ciclo
	if (UC_Etapa == 9 && M_Timer_2 == 1)
	{
		M_Timer_2 = 0;
		UC_Etapa = 14;
	}
	// Etapa final
	if (UC_Etapa == 14 && I_Llave_Tanque == 0)
	{
		UC_Etapa = 0;
		M_En_Ciclo = 0;
	}
	
	// Etapas especiales
	// Etapa 11
	if (UC_Etapa == 0 && I_Llave_Tanque == 1 && I_Desague_Manual == 1)
	{
		UC_Etapa = 11;
	}
	// Etapa 0
	if (UC_Etapa == 11 && I_Desague_Manual == 0)
	{
		UC_Etapa = 0;
	}
	
	// Acciones de cada etapa
	// Espera
	if (UC_Etapa == 0)
	{
		apagar_outs();
	}
	// Descarga manual
	if(UC_Etapa == 11)
	{
		M_Desague = 1;
	}
	// Etapa 1
	if (UC_Etapa == 1)
	{
		M_Desague = 0;
		M_Agua_Fria = 1;
	}
	// Etapa 2
	if (UC_Etapa == 2)
	{
		M_Agua_Fria = 0;
		M_Bomba_Agitador = 1;
		if (M_Activar_Timer != UC_Etapa)
		{
			M_Timer_1 = 1;
			M_Activar_Timer = UC_Etapa;
		}
		timerPulse(M_Timer_1, M_Time_1, TIMER_01, T_Enjuague);
	}
	// Etapa 3
	if (UC_Etapa == 3)
	{
		M_Timer_1 = 0;
		M_Bomba_Agitador = 0;
		M_Desague = 1;
		if (M_Activar_Timer != UC_Etapa)
		{
			M_Timer_2 = 1;
			M_Activar_Timer = UC_Etapa;
		}
		timerPulse(M_Timer_2, M_Time_2, TIMER_02, T_Descarga);
	}
	// Etapa 4
	if (UC_Etapa == 4)
	{
		M_Timer_2 = 0;
		M_Desague = 0;
		M_Agua_Caliente = 1;
	}
	// Etapa 5
	if (UC_Etapa == 5)
	{
		M_Agua_Caliente = 0;
		M_Bomba_Agitador = 1;
		if (M_Activar_Timer != UC_Etapa)
		{
			M_Timer_3 = 1;
			M_Activar_Timer = UC_Etapa;
		}
		timerPulse(M_Timer_3, M_Time_3, TIMER_03, T_Lavado);
	}
	// Etapa 6
	if (UC_Etapa == 6)
	{
		M_Timer_3 = 0;
		M_Bomba_Agitador = 0;
		M_Desague = 1;
		if (M_Activar_Timer != UC_Etapa)
		{
			M_Timer_2 = 1;
			M_Activar_Timer = UC_Etapa;
		}
		timerPulse(M_Timer_2, M_Time_2, TIMER_02, T_Descarga);
	}
	// Etapa 7
	if (UC_Etapa == 7)
	{
		M_Timer_2 = 0;
		M_Desague = 0;
		M_Agua_Fria = 1;
	}
	// Etapa 8
	if (UC_Etapa == 8)
	{
		M_Agua_Fria = 0;
		M_Bomba_Agitador = 1;
		if (M_Activar_Timer != UC_Etapa)
		{
			M_Timer_1 = 1;
			M_Activar_Timer = UC_Etapa;
		}
		timerPulse(M_Timer_1, M_Time_1, TIMER_01, T_Enjuague);
	}
	// Etapa 9
	if (UC_Etapa == 9)
	{
		M_Timer_1 = 0;
		M_Bomba_Agitador = 0;
		M_Desague = 1;
		if (M_Activar_Timer != UC_Etapa)
		{
			M_Timer_2 = 1;
			M_Activar_Timer = UC_Etapa;
		}
		timerPulse(M_Timer_2, M_Time_2, TIMER_02, T_Descarga);
	}
	// Etapa final ciclo
	if (UC_Etapa == 14)
	{
		apagar_outs();
	}
	
	// Teclado
	switch (lcd_key)               // depending on which button was pushed, we perform an action
	{
		case btnRIGHT:
		{
			if (Btn_Select == 2)
			{
				if (Pantalla == 0)
				{
					T_Enjuague_s--;
					if (T_Enjuague_s < 1)
					{
						T_Enjuague_s = 255;
					}
				}
				if (Pantalla == 1)
				{
					T_Lavado_s--;
					if (T_Lavado_s < 1)
					{
						T_Lavado_s = 255;
					}
				}
				if (Pantalla == 2)
				{
					T_Descarga_s--;
					if (T_Descarga_s < 1)
					{
						T_Descarga_s = 255;
					}
				}
			}
			break;
		}
		case btnLEFT:
		{
			if (Btn_Select == 2)
			{
				if (Pantalla == 0)
				{
					T_Enjuague_s++;
					if (T_Enjuague_s > 255)
					{
						T_Enjuague_s = 0;
					}
				}
				if (Pantalla == 1)
				{
					T_Lavado_s++;
					if (T_Lavado_s > 255)
					{
						T_Lavado_s = 0;
					}
				}
				if (Pantalla == 2)
				{
					T_Descarga_s++;
					if (T_Descarga_s > 255)
					{
						T_Descarga_s = 0;
					}
				}
			}
			break;
		}
		case btnUP:
		{
			if (Pantalla == 2)
			{
				Pantalla = 0;
			} 
			else
			{
				Pantalla++;
			}
			break;
		}
		case btnDOWN:
		{
			if (Pantalla == 0)
			{
				Pantalla = 2;
			} 
			else
			{
				Pantalla--;
			}
			break;
		}
		case btnSELECT:
		{
			Btn_Select++;
			if (Modo_Menu == 0 && Btn_Select == 1)	// Ingresa a menus
			{
				Modo_Menu = 1;
				M_En_Espera_2 = 100;
			}
			if (Modo_Menu == 1 && Btn_Select == 2)	// Modifica variables
			{
				lcd.blink();
			}
			if (Modo_Menu == 1 && Btn_Select == 3)	// Guarda variables
			{
				lcd.noBlink();
				if (Pantalla == 0)
				{
					EEPROM.write(0,T_Enjuague_s);
				}
				if (Pantalla == 1)
				{
					EEPROM.write(1,T_Lavado_s);
				}
				if (Pantalla == 2)
				{
					EEPROM.write(2,T_Descarga_s);
				}
			}
			if (Modo_Menu == 1 && Btn_Select == 4)	// Sale de menus
			{
				Modo_Menu = 0;
				M_En_Espera = 10;
				Btn_Select = 0;
			}
			break;
		}
		case btnNONE:
		{
			
			break;
		}
	}
		
	// Impresi�n en pantalla
	if(Modo_Menu == 0)
	{	
		switch (UC_Etapa)
		{
			case Descarga_Manual:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("DESC. MANUAL... ");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
			case Carga_Agua_Fria_1:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("CARGANDO FRIA...");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
			case Enjuague_1:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("ENJUAGUE...     ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("TIEMPO [s]:");
					M_En_Espera = UC_Etapa;
				
				}
				lcd.setCursor(12,1);            // move cursor to second line "1" and 0 spaces over
				lcd.print(M_Time_1);		
				break;
			}
			case Descarga_Agua_1:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("DESCARGANDO...  ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("TIEMPO [s]:");
					M_En_Espera = UC_Etapa;
				}
				lcd.setCursor(12,1);            // move cursor to second line "1" and 0 spaces over
				lcd.print(M_Time_2);
				break;
			}
			case Carga_Agua_Caliente:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("CARGANDO CAL... ");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
			case Lavado:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("LAVANDO...      ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("TIEMPO [s]:");
					M_En_Espera = UC_Etapa;
				}
				lcd.setCursor(12,1);            // move cursor to second line "1" and 0 spaces over
				lcd.print(M_Time_3);
				break;
			}
			case Descarga_Agua_2:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("DESCARGANDO...  ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("TIEMPO [s]:");
					M_En_Espera = UC_Etapa;
				}
				lcd.setCursor(12,1);            // move cursor to second line "1" and 0 spaces over
				lcd.print(M_Time_2);
				break;
			}
			case Carga_Agua_Fria_2:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("CARGANDO FRIA...");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
			case Enjuague_2:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("ENJUAGUE...     ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("TIEMPO [s]:");
					M_En_Espera = UC_Etapa;
				
				}
				lcd.setCursor(12,1);            // move cursor to second line "1" and 0 spaces over
				lcd.print(M_Time_1);
				break;
			}
			case Descarga_Agua_3:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("DESCARGANDO...  ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("TIEMPO [s]:");
					M_En_Espera = UC_Etapa;
				}
				lcd.setCursor(12,1);            // move cursor to second line "1" and 0 spaces over
				lcd.print(M_Time_2);
				break;
			}
			case En_Espera:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("EN ESPERA...    ");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
			case Alarma_Bomba_Agit:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("ALARMA!!!       ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("FALLA BOMBA/AGIT");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
			case Alarma_Llave_T:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("ALARMA!!!       ");
					lcd.setCursor(0,1);            // move cursor to second line "1" and 0 spaces over
					lcd.print("LLAVE T. CERRADA");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
			case Final_Ciclo:
			{
				if(M_En_Espera != UC_Etapa)
				{
					lcd.clear();
					lcd.print("CICLO TERMINADO!");
					M_En_Espera = UC_Etapa;
				}
				break;
			}
		}
	}
	if(Modo_Menu == 1)
	{
		switch (Pantalla)
		{
			case 0:
			{
				if(M_En_Espera_2 != Pantalla)
				{
					lcd.clear();
					lcd.print("Tiempo Enjuague:");
					M_En_Espera_2 = Pantalla;
				}
				lcd.setCursor(0,1);
				lcd.print(T_Enjuague_s);
				lcd.print(" min.");
				break;
			}
			case 1:
			{
				if(M_En_Espera_2 != Pantalla)
				{
					lcd.clear();
					lcd.print("Tiempo Lavado:  ");
					M_En_Espera_2 = Pantalla;
				}
				lcd.setCursor(0,1);
				lcd.print(T_Lavado_s);
				lcd.print(" min.");
				break;
			}
			case 2:
			{
				if(M_En_Espera_2 != Pantalla)
				{
					lcd.clear();
					lcd.print("Tiempo Descarga:");
					M_En_Espera_2 = Pantalla;
				}
				lcd.setCursor(0,1);
				lcd.print(T_Descarga_s);
				lcd.print(" min.");
				break;
			}
		}
	}
	vigia();
	escribir_outs();	// Actualiza el valor de las salidas digitales
}

// Inicializa entradas y salidas
void ini_in_out(void)
{
	// Entradas
	pinMode(I0, INPUT);
	pinMode(I1, INPUT);
	pinMode(I2, INPUT);
	pinMode(I3, INPUT);
	pinMode(I4, INPUT);
	pinMode(I5, INPUT);
	
	// Salidas
	pinMode(Q_Agua_Fria, OUTPUT);
	pinMode(Q_Bomba_Agitador, OUTPUT);
	pinMode(Q_Desague, OUTPUT);
	pinMode(Q_Agua_Caliente, OUTPUT);
	return;
}

// Lee las entradas y almacena el estado de las mismas
void leer_in(void)
{
	I_Inicio_Parada = digitalRead(I0);
	I_Nivel = digitalRead(I1);
	I_Bomba_Agitador = digitalRead(I2);
	I_Desague_Manual = digitalRead(I3);
	I_Llave_Tanque = digitalRead(I4);
	I_Lav_Auto = digitalRead(I5);
	return;
}

// Escribe las salidas
void escribir_outs(void)
{
	digitalWrite(Q_Agua_Fria, M_Agua_Fria);
	digitalWrite(Q_Bomba_Agitador, M_Bomba_Agitador);
	digitalWrite(Q_Desague, M_Desague);
	digitalWrite(Q_Agua_Caliente, M_Agua_Caliente);
	return;
}

//  Apaga todas las salidas
void apagar_outs(void)
{
	M_Agua_Fria = 0;
	M_Bomba_Agitador = 0;
	M_Desague = 0;
	M_Agua_Caliente = 0;
	return;
}

// Temporizador tipo pulso
unsigned long timerPulse(boolean &timeractive, unsigned long &timeactual, unsigned long &timerState, unsigned long timerPeriod)
{
	if ((timeractive == 0) & (timerState == 0))				// Timer is either not triggered or finished
	{														
		timerState = 0;
		timeactual = 0;										// Clear timerState (0 = 'not started')
	}
	else                                                    // Timer is enabled
	{														
		if (timerState == 0)								// Timer hasn't started counting yet
		{													
			timerState = millis();							// Set timerState to current time in milliseconds
			timeractive = 0;
			timeactual = 0;									// Result = 'not finished' (0)
		}
		else                                                // Timer is active and counting
		{												
			if (millis() > (timerState + timerPeriod))		// Timer has finished
			{	
				timeractive = 1;							// Pulse = 'finished' (0)
				timerState = 0;
			}
			else
			{												// Timer has not finished
				timeractive = 0;							// Pulse = 'Active' (1)
			}
			timeactual = (millis() - timerState) / 1000;
		}	
	}
	return(timeactual);										// Return result (1 = 'finished',
															// 0 = 'not started' / 'not finished')
}

// Vigia detecci�n de alarmas
void vigia(void)
{
	if (I_Bomba_Agitador == 1)
	{
		M_En_Alarma = 1;
		UC_Etapa = 12;
		apagar_outs();
	}
	if (UC_Etapa != 0 && I_Llave_Tanque == 0 && M_En_Alarma == 0)
	{
		M_En_Alarma = 2;
		UC_Etapa = 13;
		apagar_outs();
	}
	if (M_En_Alarma == 1 && I_Bomba_Agitador == 0 && UC_Etapa == 12)
	{
		UC_Etapa = 0;
		M_En_Ciclo = 0;
		M_En_Alarma = 0;
		M_En_Espera = 10;
		M_Activar_Timer = 0;
		M_Timer_1 = 0;
		M_Timer_2 = 0;
		M_Timer_3 = 0;
		M_Time_1 = 0;
		M_Time_2 = 0;
		M_Time_3 = 0;
		TIMER_00 = 0;  // Variable to hold elapsed time for Timer 0
		TIMER_01 = 0;  // Variable to hold elapsed time for Timer 1
		TIMER_02 = 0;  // Variable to hold elapsed time for Timer 2
		TIMER_03 = 0;  // Variable to hold elapsed time for Timer 3
	}
	if (M_En_Alarma == 2 && I_Llave_Tanque == 1 && UC_Etapa == 13)
	{
		UC_Etapa = 0;
		M_En_Ciclo = 0;
		M_En_Alarma = 0;
		M_En_Espera = 10;
		M_Activar_Timer = 0;
		M_Timer_1 = 0;
		M_Timer_2 = 0;
		M_Timer_3 = 0;
		M_Time_1 = 0;
		M_Time_2 = 0;
		M_Time_3 = 0;
		TIMER_00 = 0;  // Variable to hold elapsed time for Timer 0
		TIMER_01 = 0;  // Variable to hold elapsed time for Timer 1
		TIMER_02 = 0;  // Variable to hold elapsed time for Timer 2
		TIMER_03 = 0;  // Variable to hold elapsed time for Timer 3
	}
	return;
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

int Boton_Con_Rebote(void)
{
	aux_key_1 = read_LCD_buttons();
	delay(85);
	aux_key_2 = read_LCD_buttons();
	if (aux_key_1 == aux_key_2)
	{
		return aux_key_2;
	}
	else
	{
		return btnNONE;
	}
	/*
	if (M_Activar_Timer_2 != aux_key_1)
	{
		M_Timer_4 = 1;
		M_Activar_Timer = aux_key_1;
	}
	timerPulse(M_Timer_4, M_Time_4, TIMER_04, T_Rebote);
	if (M_Time_4 == T_Rebote)
	{
		aux_key_2 = read_LCD_buttons();
	}
	if (aux_key_1 == aux_key_2)
	{
		return aux_key_2;
	} 
	else
	{
		return btnNONE;
	}
	*/
}
