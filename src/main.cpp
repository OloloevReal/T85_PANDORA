#include "Arduino.h"

#include <avr/io.h>
#include <util/delay.h>
// #include <avr/interrupt.h>
 #include <avr/sleep.h>
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

bool flag_b = false;
uint8_t ack_t = 0;

bool calibration();
void flashPIN(uint8_t _pin, uint8_t n, uint8_t mult = 1, bool _post_delay = false);

void setup(){
    Serial.begin(9600);
    
    DDRB |= (1 << PIN_LED1) | (1 << PIN_VALET) | (1 << PIN_BUTTON); // PIN OUTPUT
    PORTB &= ~_BV(PIN_LED1);  // PIN STATE LOW
    PORTB &= ~_BV(PIN_VALET); // PIN STATE LOW
    PORTB |= _BV(PIN_BUTTON); // HIGHT

    if(!digitalRead(PIN_BUTTON)){
        Serial.println("Storage clearing");
       _storage.clear();
    }

    if(!_storage.get(_data)){
        Serial.println("Storage is empty");
        _data._v = 1325;
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

    flashPIN(PIN_LED1, 1, 4, false);
    flag_b = calibration();
    if(!flag_b){
        Serial.println("Calibration failed!");
        flashPIN(PIN_LED1, 5, 2, false);
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_mode();
    }
}

void flashPIN(uint8_t _pin, uint8_t n, uint8_t mult = 1, bool _post_delay = false){
    for (size_t i = 0; i < n; i++)
    {
        PORTB ^=_BV(_pin);
        delay((100 * mult)); // TODO: change to _delay_ms
        PORTB ^=_BV(_pin);
        delay((100 * mult));
    }
    if(_post_delay){
        _delay_ms(1000);
    }
}

uint8_t pushValet(uint8_t num){
        Serial.printf("%d ", num);
        flashPIN(PIN_VALET, num, 1);
        _delay_ms(500);
        uint8_t ack = _sensor->GetImpulse();
        Serial.printf("(%d) ", ack);
        return ack;
}



bool calibration(){
    _delay_ms(100);
    if(!_sensor->GetValueBool()){
        return pushValet(2) > 0?true:false;
    }
    return false;
}

void loop(){
    if (_data._m != WORK)
    {
        if(_data._m == STOP){
            Serial.printf("PIN: %d\r\n", _digs->GetValue());
            //print result
            for(uint8_t i = 0; i < sizeof(_digs->nn);i++){
                flashPIN(PIN_LED1,_digs->nn[i], 2, true);
            }
        }
        _delay_ms(5000);
        return;
    }

        Serial.printf("->: %d\r\n", _digs->Next());
        flag_b = true;

        for(uint8_t i = 0; i < sizeof(_digs->nn); i++){
            ack_t = pushValet(_digs->nn[i]);
            flashPIN(PIN_LED1, ack_t == 0?3:ack_t, 2);
            flag_b &= ack_t>0?true:false;
        }

        Serial.printf("%s\r\n",flag_b?"OK":"NOK");

        if(ack_t == 2){
            _data._m = STOP;
        }else{
            delay(5000);
        }
        
        _data._m = _digs->GetValue() == 9999?STOP:_data._m;

        _data._v = (uint16_t)_digs->GetValue();
        _storage.add(_data);
}