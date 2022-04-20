#ifndef _SENSOR_h
#define _SENSOR_h

#include "Arduino.h"

#define TIMEOUT 4 // 5 sec timout default
#define _very_short_impulse_t 200
#define _short_impulse_t 1200
#define _lowLevel 970
#define _delay_default 10

	/*
	True -> False: 11110000
	False -> True: 00000111
	*/

//static uint8_t getADC(uint8_t pin);

class Sensor_Real
{
private:
    uint8_t _pin;
    uint8_t _adc;
    // int _no_of_samples = 64;
    // int _delay_default = 50;

public:
    Sensor_Real(uint8_t pin, uint8_t adc){
        _pin = pin;
        _adc = adc;
    };
    /*
    0 - 0
    1 - short impuls
    2 - long impuls
    */

    // uint16_t millis16(){
    //     return millis() & 0xFFFF;
    // }

    uint8_t GetImpulse(uint8_t timeout = TIMEOUT){
        unsigned long start_t = millis();
        unsigned long start_imp_t = 0;
        bool value = false;
        while(millis() - start_t < timeout * 1000){
            // false -> true
            if(!value && GetValueBool()){
                value = true;
                start_imp_t = millis();
            }

            // true -> false
            if(value && !GetValueBool()){
                value = false;
                //ignore very short impulse
                if(millis() - start_imp_t > _very_short_impulse_t){
                    break;
                }else{
                    start_imp_t = 0;
                }
                
            }
            delay(_delay_default);
        }

        if(start_imp_t == 0 ){
            return 0;
        }else{
            if(millis() - start_imp_t <= _short_impulse_t){
                return 1;
            }else{
                return 2;
            }
        }

    };

    void Init() {
        pinMode(_pin, INPUT_PULLUP);
    };

    uint16_t GetValue(){
        return analogRead(_adc);
    }

    bool GetValueBool(){
        return GetValue()<_lowLevel?true:false;
    }
};

// static uint8_t getADC(uint8_t pin){
//     switch (pin)
//     {
//     case 2:
//         return 1;
//     case 3:
//         return 3;
//     case 4:
//         return 2;
//     case 5:
//         return 0;
    
//     default:
//         return -1;
//     }
// }

#endif

    // uint8_t GetImpulse(uint8_t num = 1, uint8_t timeout = TIMEOUT)override{
    //     long start_t = millis();
    //     uint8_t num_t = 0;
    //     uint16_t value_t = 0;

    //     while((millis()-start_t < timeout * 1000) && (num_t < num)){ //|| num_t < num
    //         value_t = GetValue();
    //         //Serial.printf("value_t: %d\t%lu\r\n", value_t, millis()-start_t);
    //         if(value_t < _lowLevel){
    //             num_t++;
    //         }
    //         delay(_delay_default);
    //     }
    //     return num_t;
    // };