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

#define PIN_BUTTON PIN_B0

#define PIN_LED1 PIN_B4

#define PIN_VALET PIN_B1

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
    
    DDRB |= (1 << PIN_LED1) | (1 << PIN_VALET) | (1 << PIN_BUTTON); // OUTPUT
    PORTB &= ~_BV(PIN_LED1);  // LOW
    PORTB &= ~_BV(PIN_VALET); // LOW
    PORTB |= _BV(PIN_BUTTON); // HIGHT

    // pinMode(PIN_LED1, OUTPUT);
    // digitalWrite(PIN_LED1, LOW);

    // pinMode(PIN_VALET, OUTPUT);

    // pinMode(PIN_BUTTON, OUTPUT);

    if(!digitalRead(PIN_BUTTON)){
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

void flashLed(uint8_t _pin, uint8_t n, uint8_t mult = 1, bool _post_delay = true){
    for (size_t i = 0; i < n; i++)
    {
        PORTB ^=_BV(_pin);
        delay((150 * mult)); // TODO: change to _delay_ms
        PORTB ^=_BV(_pin);
        delay((100 * mult));
    }
    if(_post_delay){
        _delay_ms(1000);
    }
}

uint8_t pushValet(uint8_t num){
        Serial.printf("%d ", num);
        flashLed(PIN_VALET, num, 1, false);
        //Serial.printf("Impuls: %d\r\n", _sensor->GetImpuls());
        //_delay_ms(500);
        uint8_t ack = _sensor->GetImpulse();
        Serial.printf("(%d) ", ack);
        return ack;
}

void loop(){
    if (_data._m != WORK)
    {
        if(_data._m == STOP){
            //print result
            flashLed(PIN_LED1,_digs->n0, 2);
            //_delay_ms(t_delay);
            flashLed(PIN_LED1,_digs->n1, 2);
            //_delay_ms(t_delay);
            flashLed(PIN_LED1,_digs->n2, 2);
            //_delay_ms(t_delay);
            flashLed(PIN_LED1,_digs->n3, 2, false);
        }
        _delay_ms(5000);
        return -1;
    }
        //flashLed(PIN_LED1, 2, 2, false);
        Serial.printf("->: %d\r\n", _digs->Next());
        //Serial.printf("PIN0: %d\r\n", _sensor->GetValue());
        
        flashLed(PIN_LED1, 1, 2, false);
        pushValet(_digs->n0);

        flashLed(PIN_LED1, 1, 2, false);
        pushValet(_digs->n1);

        flashLed(PIN_LED1, 1, 2, false);
        pushValet(_digs->n2);

        flashLed(PIN_LED1, 1, 2, false);
        uint8_t ack = pushValet(_digs->n3);

        Serial.println();

        // Serial.print("Impulse: "); 
        // uint8_t _value = _sensor->GetImpulse();
        // Serial.println(_value);
        if(ack == 2){
            _data._m = STOP;
        }else{
            delay(4000);
        }
        
        _data._m = _digs->GetValue() == 9999?STOP:_data._m;
        // _data._m = _digs->GetValue() == 3945?STOP:WORK;

        _data._v = (uint16_t)_digs->GetValue();
        _storage.add(_data);
        //_delay_ms(1000);
}