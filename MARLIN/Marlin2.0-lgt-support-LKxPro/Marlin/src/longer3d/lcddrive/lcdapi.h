#pragma once

#if ENABLED(LGT_LCD_TFT)
#include "stdint.h"

#define LCD_WIDTH 320u
#define LCD_HIGHT 240u
#define LCD_PIXELS_COUNT (LCD_WIDTH * LCD_HIGHT)

// color definition for lcd
#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

#define DARKBLUE    0X01CF	
#define LIGHTBLUE   0X7D7C
#define GRAYBLUE    0X5458 
#define GRAY        0X8430//0xF7DE


#define IMAGE_BUFF_SIZE  4096   // store image data from spiflash
// #define SLOW_SHOW_IMAGE      // using slowly io write method  

struct imageHeader {
   uint8_t scan;
   uint8_t gray;
   uint16_t w;
   uint16_t h;
   uint8_t is565;
   uint8_t rgb;
}; 

class LgtLcdApi {
public:
    LgtLcdApi();
    uint8_t init();
    inline void clear(uint16_t color=WHITE) {
        fill(0, 0, LCD_WIDTH-1, LCD_HIGHT-1, color);
    }
    void fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
    void backLightOff();
    void backLightOn();
    inline void setColor(uint16_t c) 
    {
        m_color = c;
    }
    inline void setBgColor(uint16_t c) 
    {
        m_bgColor = c;
    }

    inline uint16_t lcdId()
    {
        return m_lcdID;
    }

    void print(uint16_t x, uint16_t y, const char *text);
    void showImage(uint16_t x_st, uint16_t y_st, uint32_t addr);
    void showRawImage(uint16_t xsta,uint16_t ysta,uint16_t width,uint16_t high, uint32_t addr);
    void drawCross(uint16_t x, uint16_t y, uint16_t color);

    inline void drawHVLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
    {
        #define SWAP(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b)))
        if (x1 > x2)
            SWAP(x1, x2);
        if (y1 > y2)
            SWAP(y1, y2);
        fill(x1, y1, x2, y2, m_color);
    }

    inline void drawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
    {
        fill(x1, y1, x2, y1, m_color);
        fill(x2, y1, x2, y2, m_color);
        fill(x1, y2, x2, y2, m_color);
        fill(x1, y1, x1, y2, m_color);
    }

public:
    void prepareWriteRAM();
    void setCursor(uint16_t Xpos, uint16_t Ypos);
    void setWindow(uint16_t Xmin, uint16_t Ymin, uint16_t XMax=LCD_WIDTH-1, uint16_t Ymax=LCD_HIGHT-1);    

private:
    uint16_t m_lcdID;
public:
    uint16_t m_color;
    uint16_t m_bgColor;

};

extern LgtLcdApi lgtlcd;

#endif