#ifndef _STORAGE_h
#define _STORAGE_h

#include <Arduino.h>
#include <EEPROM.h>

uint8_t crc_8(unsigned char *input_str, size_t num_bytes);

enum Mode{
    FIRST = 0,
    WORK = 1,
    STOP = 2,
};

//const __FlashStringHelper* getMode_co_str(Mode mode);


struct Data
{
    uint16_t _v;
    Mode _m;
};

template<class T>
class EEPROMStorage
{
    /*
    Кольцевой буфер в памяти для хранения 1го элемента класса Т. Чтобы превысить максимальное
    число записи в EEPROM (100 000). 

    blocks - кол-во копий Т (размер кольцевого буфера).

    После блоков следует массив флагов для обозначения самого последнего блока буфера.
    Флаг - crc8 соответствующего блока. 
    Добавление значения - запись в блок + crc блока в следующей ячейке массива флагов.
    Если crc=0, то crc=1.

    При перезагрузке микроконтроллера текущий блок восстановиться и буфер продолжит работу.

    |xxxx|xxxx|xxxx|xxxx|crc|0|0|0|
    */

public: 
    explicit EEPROMStorage(const uint8_t _blocks, const uint8_t _start_addr = 0);
    void add(const T &element);
    bool get(T &element);
    bool get_block(const uint8_t block, T &element);
    bool clear();
    
    uint16_t size();    
private:

    uint8_t start_addr;
    uint8_t activeBlock;
    uint8_t blocks;

    uint8_t elementSize;

    uint16_t flag_shift;
};


#endif
