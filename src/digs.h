#ifndef DIGS_h
#define DIGS_h
#include "Arduino.h"

// #define _max_value 9999
// #define _min_value 1111

    struct digs
    {
    private:
        int _max_value = 9999;
        int _min_value = 1111;
        int _value = 0;

        bool isZero(){
            return nn[0] == 0 || nn[1] == 0 || nn[2] == 0 || nn[3] == 0?true:false;
        }
    public:
        uint8_t nn[4];
        // uint8_t n0;
        // uint8_t n1;
        // uint8_t n2;
        // uint8_t n3;

        void Parse(int value){
            _value = value;
            // n3 = ;
            // n2 = ;
            // n1 = ;
            // n0 = ;
            nn[0] = value / 1000 % 10;
            nn[1] = value / 100 % 10;
            nn[2] = value / 10 % 10;
            nn[3] = value % 10;

        };

        void Print(){
            Serial.printf("%d %d %d %d\r\n", nn[0], nn[1], nn[2], nn[3]);
        };

        int GetValue(){
            return _value;
        };

        int Next(){
            if(_value == 0){
                _value = _min_value;
                Parse(_value);
                return GetValue();
            }
            _value++;
            for(int i = _value; i <= _max_value; i++){
                Parse(i);
                
                if(!isZero()){
                    _value = i;
                    return GetValue();
                }else{
                    Print();
                }
            }
            return _value;
        };
    };
    
#endif