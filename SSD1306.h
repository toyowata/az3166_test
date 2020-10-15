#ifndef SSD1306_H
#define SSD1306_H
 
#include "SSD1308.h"
 
class SSD1306 : public SSD1308 {
public:
    SSD1306(I2C *i2c, uint8_t address) : SSD1308(i2c,address) {
        char databytes[4] = {COMMAND_MODE, 0x8d, //charge pump enable
                             COMMAND_MODE, 0x14  //charge pump ON
                            };
        i2c->write(address, databytes, 4);
        setDisplayFlip(false, false);
    }
};
#endif
