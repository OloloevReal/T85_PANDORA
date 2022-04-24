#include "Arduino.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "Storage.h"
#include "digs.h"
#include "sensor.h"

#define VERSION "0.0.2"

#define PIN_BUTTON PIN_B0
#define PIN_LED1 PIN_B4
#define PIN_VALET PIN_B1

#define PIN_SENSOR PIN_B5
#define ADC_SENSOR 0

#define CALIBRATION
#define POWER_SAVE

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
void SleepAndRestart(uint8_t time_s, bool restart = true);
void SleepDisable();

void setup(){
#ifdef POWER_SAVE
    SleepDisable();
    MCUSR = 0;
#endif
    Serial.begin(9600);
    Serial.printf("\r\nVer: %s\r\n", VERSION);

    DDRB |= (1 << PIN_LED1) | (1 << PIN_VALET) | (1 << PIN_BUTTON); // PIN OUTPUT
    PORTB &= ~_BV(PIN_LED1);  // PIN STATE LOW
    PORTB &= ~_BV(PIN_VALET); // PIN STATE LOW
    PORTB |= _BV(PIN_BUTTON); // HIGHT

    if(!digitalRead(PIN_BUTTON)){
        Serial.println("Clearing");
       _storage.clear();
    }

    if(!_storage.get(_data)){
        //Serial.println("Storage is empty");
        _storage.add(_data);
    }
    if(_data._v == 0 || _data._m == FIRST){
        //first run
        //_data._v = 0;
        _data._v = 2320;
        _data._m = WORK;
    }

    _digs->Parse(_data._v);

    // Serial.printf(("Stored mode: %S\r\n"), getMode_co_str( _data._m));
    // Serial.printf(("Stored value: %d\r\n"), _data._v);
    _sensor->Init();

    delay(1000);
    Serial.printf("Ref val: %d\r\n", _sensor->GetValue());

#ifdef CALIBRATION
    if(_data._m == WORK){
        flashPIN(PIN_LED1, 1, 4, true);
        flag_b = calibration();
        if(!flag_b){
            Serial.println("Calibration failed!");
            flashPIN(PIN_LED1, 5, 2, false);
#ifdef POWER_SAVE
            SleepAndRestart(WDTO_8S);
#else
            _delay_ms(8000);
#endif
        }
        PORTB |= _BV(PIN_LED1);
#ifdef POWER_SAVE
        SleepAndRestart(WDTO_8S | WDTO_2S, false);
#else
        _delay_ms(10000);
#endif
        PORTB &= ~_BV(PIN_LED1);
    }
#endif
}

#ifdef POWER_SAVE
ISR(WDT_vect) {
}


void SleepDisable(){
    wdt_disable();
    cli();
}

void SleepAndRestart(uint8_t time_s, bool restart){
    wdt_disable();
    
    wdt_enable(time_s); //restart after sec
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    if(!restart){
        WDTCR |= _BV(WDE) | _BV(WDIE);
    }else{
        WDTCR &= ~_BV(WDIE);
    }
    sleep_mode();
    wdt_disable();
    cli();
}
#endif

void flashPIN(uint8_t _pin, uint8_t n, uint8_t mult, bool _post_delay){
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
        delay(500);
        uint8_t ack = _sensor->GetImpulse();
        Serial.printf("(%d) ", ack);
        return ack;
}

bool calibration(){
    delay(200);
    if(!_sensor->GetValueBool(false)){
        delay(200);
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
                flashPIN(PIN_LED1,_digs->nn[i], 3, true);
            }
        }
        if(!digitalRead(PIN_BUTTON)){
            _data._m = WORK;
            _storage.add(_data);
        }
#ifdef POWER_SAVE
        SleepAndRestart(WDTO_4S|WDTO_2S);
#else
        return;
#endif
    }

    Serial.printf("->: %d\r\n", _digs->Next());
    flag_b = true;

    for(uint8_t i = 0; i < sizeof(_digs->nn); i++){
        ack_t = pushValet(_digs->nn[i]);
        flashPIN(PIN_LED1, ack_t == 0?3:ack_t, 1);
        flag_b &= ack_t>0?true:false;
    }

    Serial.printf("%s\r\n",flag_b?"OK":"NOK");
    
    _data._m = _digs->GetValue() == 9999 || ack_t == 2 ?STOP:_data._m;

    _data._v = (uint16_t)_digs->GetValue();
    _storage.add(_data);
#ifdef POWER_SAVE
    SleepAndRestart(WDTO_4S, false);
#else
    _delay_ms(4000);
#endif
}