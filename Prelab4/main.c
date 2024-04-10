/*
 * Lab 4 Progra de Microcontroladores
 * Created: 7/04/2024 18:52:36
 * Author : Juan Fernando Maldonado Menéndez
 * Descripción: Contador de 8 bits con leds en C con interrupciones de botones.
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>



volatile uint8_t contador = 0; // Inicializo mi contador.
volatile	uint8_t valoradc = 0; //Variable donde guardare el valor ADC.
volatile	uint8_t bmenosig = 0; //Variable donde guardo los 4 bits menos significativos.
volatile	uint8_t bmasig   = 0; //Variable donde guardo los 4 bits mas significativos.
volatile    uint8_t transistor = 0; //Variable donde dirigo si enciendo leds, contador o displays.


//***************************************************************************************************************************************************************
// Setup:
//***************************************************************************************************************************************************************
void setup(void) {
	cli();
	// Configura los pines de los botones como entrada con pull-up
	UCSR0B = 0;
	DDRC &= ~(1 << PINC0); //Indico que en este pin existira una entrada para PC0
	PORTC |= (1 << PINC0);
	DDRC &= ~(1 << PINC1); //Indico que en este pin existira una entrada para PC1
	PORTC |= (1 << PINC1);
	DDRD = 0xFF;           //Seteo mi puerto D todos salidas.
	PORTD = contador;      //Le asigno que siempre en el puerto D me muestre el contador.
	DDRB = 0xFF;           //Habilito mi puerto B como salidas.
	PORTB = 0;             //Apago mi puerto B.
	// Habilita las interrupciones externas
	PCICR |= (1 << PCIE1); // Habilita la interrupción PCINT8-14
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9); // Habilita interrupción PC0 y PC1
	sei();
}

uint8_t Milista[] = {0b01000000, 0b01111001, 0b00100100, 0b00110000, 0b00011001, 0b00010010, 0b00000010, 0b01111000, 0b00000000, 0b00010000, 0b10001000, 0b10000011, 0b11000110
	, 0b10100001 , 0b10000110, 0b10001110}; //Creo mi lista con la configuracion del display de 7 segmentos de anodo comun de 2 digitos.
	
void InitADC(void){
	ADMUX |= (1 << REFS0);      //Estoy configurando a que funcione con AVCC.
	ADMUX &= ~(1 << REFS1);
	//Justificacion a la izquierda.
	ADMUX |= (1 << ADLAR); 
	//Configuración del canal 7 ya que allí esta mi potenciómetro.
	ADMUX |= (1 << MUX2)|(1 << MUX1)|(1 << MUX0); 
	ADCSRA = 0;
	//Habilitamos la bandera del ADC
	ADCSRA |= (1 << ADIE);
	//Habilitamos el prescaler a 128.
	ADCSRA |= (1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0);
	//Habilitamos el ADC
	ADCSRA |= (1 << ADEN);
	
	
}
//***************************************************************************************************************************************************************
// Loop principal:
//***************************************************************************************************************************************************************
int main(void) {
	setup(); // Inicializa todo.
	InitADC();//Configuro mi ADC.
	while (1) {
		ADCSRA |= (1<<ADSC);   //Habilito la interrupcion de ADC.
		_delay_ms(100);        //Pongo un retraso de 100 ms.
		transistor = 1;        //Mi transistor encendera el pin 0 del puerto B.
		PORTB = transistor;
		PORTD = contador;      // Mostrare el contador en el puerto B.
		if (contador == valoradc){ //Si el valor del ADC es igual al de contador, mantendre el orden que tenia el transistor y solo encendere el pin 3 para el led de alarma
			PORTB = (transistor + 8);
		}
	}
	return 0;
}
//***************************************************************************************************************************************************************
// Interrupciones:
//***************************************************************************************************************************************************************
ISR(PCINT1_vect) {
	if (!(PINC & (1 << PINC0))) {    
		_delay_ms(150);             //Espero 150 milisegundos si el boton se apachó, es decir, si se mandó un 0
		if (!(PINC & (1 << PINC0))) {
			contador++;             // Sumo 1 al contador, si este es mayor a 255, lo reinicio.
			if (contador > 255)     
				contador = 0;
		}
	}

	// Verifica si el botón de decremento está presionado
	if (!(PINC & (1 << PINC1))) { 
		_delay_ms(150);             // Espero 150 milisegundos si el segundo botón se apachó, si definivamente se apachó, quito 1 al contador.
		if (!(PINC & (1 << PINC1))) {
			transistor = 1;
			PORTB = transistor; 
			contador--;             // Si el contador es menor a 0, hago underflow.
			if (contador < 0) 
				contador = 255; 
				PORTD = contador;
		}
	}
}

ISR(ADC_vect){
    valoradc = ADCH;              //Le mando el valor de ADC pero los 8 bits mas significativos a la izquierda a la variable valoradc.
	bmenosig = valoradc & 0b00001111; //Hago un and para eliminar los 4 digitos superiores.
	bmasig   = (valoradc >> 4);       //Corro los 4 digitos superiores de valoradc para volverlos los 4 menos significativos de 8 bits.
			_delay_ms(100);           //Retraso de 100 ms.
			transistor = 2;           // Habilito mi variable en 2 ya que encendere el pin1 del puerto B a donde va mi primer display.
			PORTB = transistor;
			PORTD = Milista[bmenosig]; //Muestro el valor que tengo de los bits menos significativos en la lista.
			_delay_ms(100);            //Retraso de 100 ms.
			transistor = 4;            //Habilito mi variable en 4 ya que encendere solo el pin2 del puerto B a donde va mi segundo display.
			PORTB = transistor;
			PORTD = Milista[bmasig];   //Muestro el valor que tengo de los bits mas significativos en la lista.
	ADCSRA |= (1<<ADIF);               //Apago o me salgo de la interrupcion.
	
}