#include "Arduino.h"

#include <avr/io.h>
#include <util/delay.h>
// #include <avr/interrupt.h>
// #include <avr/sleep.h>
// #include <avr/wdt.h>

#include "Storage.h"
#include "digs.h"
#include "sensor.h"

#define VERSION "0.0.1"
#define LED1 PIN_B1
#define PIN_SENSOR PIN_B5
#define ADC_SENSOR 0

static EEPROMStorage<Data> _storage(100);
struct Data _data = {2, FIRST};


//PORTB |= (1 << PB0); Pin 0 ON
//PORTB &= ~(1 << PB0); Pin 0 OFF
//PORTB ^=(1<<PB0); Inversion

digs *_digs = new digs();
Sensor_Real * _sensor = new  Sensor_Real(PIN_SENSOR, ADC_SENSOR);

void setup(){
    Serial.begin(9600);
    
    DDRB |= (1 << DDB1) | (1 << DDB0);
    PORTB &= ~(1 << PB1);

    pinMode(PIN_B0, OUTPUT);
    digitalWrite(PIN_B0, HIGH);
    if(!digitalRead(PIN_B0)){
        Serial.println("Storage clearing");
       _storage.clear();
    }

    if(!_storage.get(_data)){
        Serial.println("Storage is empty");
        _storage.add(_data);
    }
    if(_data._v == 0 || _data._m == FIRST){
        //first run
        _data._v = 0;
        _data._m = WORK;
    }

    _digs->Parse(_data._v);

    // Serial.printf(("Stored mode: %S\r\n"), getMode_co_str( _data._m));
    // Serial.printf(("Stored value: %d\r\n"), _data._v);
    _sensor->Init();
}

void flashLed(uint8_t v, uint8_t mult = 1, bool _delay = true){
    for (size_t i = 0; i < v; i++)
    {
        PORTB ^=(1<<PB1);
        delay((300 * mult)); // TODO: change to _delay_ms
        PORTB ^=(1<<PB1);
        delay((200 * mult));
    }
    if(_delay){
        _delay_ms(1000);
    }
}

void pushValet(uint8_t num){
        Serial.printf(F("%d "), num);
        flashLed(num, 1, false);
        //Serial.printf("Impuls: %d\r\n", _sensor->GetImpuls());
        _delay_ms(500);
}

void loop(){
    if (_data._m != WORK)
    {
        if(_data._m == STOP){
            //print result
            flashLed(_digs->n0, 2);
            //_delay_ms(t_delay);
            flashLed(_digs->n1, 2);
            //_delay_ms(t_delay);
            flashLed(_digs->n2, 2);
            //_delay_ms(t_delay);
            flashLed(_digs->n3, 2, false);
        }
        _delay_ms(5000);
        return -1;
    }
    
        Serial.printf("->: %d\r\n", _digs->Next());
        //Serial.printf("PIN0: %d\r\n", _sensor->GetValue());
        
        pushValet(_digs->n0);
        pushValet(_digs->n1);
        pushValet(_digs->n2);
        pushValet(_digs->n3);

        Serial.println();

        Serial.print("Impulse: "); 
        uint8_t _value = _sensor->GetImpulse();
        Serial.println(_value);
        if(_value == 2){
            _data._m = STOP;
        }
        
        _data._m = _digs->GetValue() == 9999?STOP:_data._m;
        // _data._m = _digs->GetValue() == 3945?STOP:WORK;

        _data._v = (uint16_t)_digs->GetValue();
        _storage.add(_data);
        _delay_ms(1000);
}