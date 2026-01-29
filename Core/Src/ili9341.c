#include "ili9341.h"
#include <stdlib.h>

extern SPI_HandleTypeDef hspi5;

// ILI9341 commands
#define ILI9341_SWRESET     0x01
#define ILI9341_SLPOUT      0x11
#define ILI9341_DISPON      0x29
#define ILI9341_CASET       0x2A
#define ILI9341_PASET       0x2B
#define ILI9341_RAMWR       0x2C
#define ILI9341_MADCTL      0x36
#define ILI9341_COLMOD      0x3A

static void ILI9341_WriteCommand(uint8_t cmd);
static void ILI9341_WriteData(uint8_t data);
static void ILI9341_WriteData16(uint16_t data);
static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

static void ILI9341_WriteCommand(uint8_t cmd) {
    LCD_DC_LOW();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi5, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void ILI9341_WriteData(uint8_t data) {
    LCD_DC_HIGH();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi5, &data, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void ILI9341_WriteData16(uint16_t data) {
    uint8_t buffer[2];
    buffer[0] = data >> 8;
    buffer[1] = data & 0xFF;
    LCD_DC_HIGH();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi5, buffer, 2, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ILI9341_WriteCommand(ILI9341_CASET);
    ILI9341_WriteData16(x0);
    ILI9341_WriteData16(x1);
    
    ILI9341_WriteCommand(ILI9341_PASET);
    ILI9341_WriteData16(y0);
    ILI9341_WriteData16(y1);
    
    ILI9341_WriteCommand(ILI9341_RAMWR);
}

void ILI9341_Init(void) {
    // Hardware reset
    LCD_RST_HIGH();
    HAL_Delay(5);
    LCD_RST_LOW();
    HAL_Delay(20);
    LCD_RST_HIGH();
    HAL_Delay(150);
    
    // Software reset
    ILI9341_WriteCommand(ILI9341_SWRESET);
    HAL_Delay(150);
    
    // Display off
    ILI9341_WriteCommand(0x28);
    
    // Power control A
    ILI9341_WriteCommand(0xCB);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);
    
    // Power control B
    ILI9341_WriteCommand(0xCF);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);
    
    // Driver timing control A
    ILI9341_WriteCommand(0xE8);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);
    
    // Driver timing control B
    ILI9341_WriteCommand(0xEA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    
    // Power on sequence control
    ILI9341_WriteCommand(0xED);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);
    
    // Pump ratio control
    ILI9341_WriteCommand(0xF7);
    ILI9341_WriteData(0x20);
    
    // Power control 1
    ILI9341_WriteCommand(0xC0);
    ILI9341_WriteData(0x23);
    
    // Power control 2
    ILI9341_WriteCommand(0xC1);
    ILI9341_WriteData(0x10);
    
    // VCOM control 1
    ILI9341_WriteCommand(0xC5);
    ILI9341_WriteData(0x3E);
    ILI9341_WriteData(0x28);
    
    // VCOM control 2
    ILI9341_WriteCommand(0xC7);
    ILI9341_WriteData(0x86);
    
    // Memory access control
    ILI9341_WriteCommand(ILI9341_MADCTL);
    ILI9341_WriteData(0x88);  // Changed from 0x48 to 0x88 to flip the display
    
    // Pixel format
    ILI9341_WriteCommand(ILI9341_COLMOD);
    ILI9341_WriteData(0x55); // 16-bit color
    
    // Frame rate control
    ILI9341_WriteCommand(0xB1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x18);
    
    // Display function control
    ILI9341_WriteCommand(0xB6);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x27);
    
    // Enable 3G
    ILI9341_WriteCommand(0xF2);
    ILI9341_WriteData(0x00);
    
    // Gamma set
    ILI9341_WriteCommand(0x26);
    ILI9341_WriteData(0x01);
    
    // Positive gamma correction
    ILI9341_WriteCommand(0xE0);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);
    
    // Negative gamma correction
    ILI9341_WriteCommand(0xE1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);
    
    // Sleep out
    ILI9341_WriteCommand(ILI9341_SLPOUT);
    HAL_Delay(120);
    
    // Display on
    ILI9341_WriteCommand(ILI9341_DISPON);
    HAL_Delay(50);
}

void ILI9341_FillScreen(uint16_t color) {
    ILI9341_SetAddressWindow(0, 0, ILI9341_WIDTH - 1, ILI9341_HEIGHT - 1);
    
    LCD_DC_HIGH();
    LCD_CS_LOW();
    
    uint8_t colorHigh = color >> 8;
    uint8_t colorLow = color & 0xFF;
    
    for(uint32_t i = 0; i < ILI9341_WIDTH * ILI9341_HEIGHT; i++) {
        uint8_t buffer[2] = {colorHigh, colorLow};
        HAL_SPI_Transmit(&hspi5, buffer, 2, HAL_MAX_DELAY);
    }
    
    LCD_CS_HIGH();
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if(x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
    
    ILI9341_SetAddressWindow(x, y, x, y);
    ILI9341_WriteData16(color);
}

void ILI9341_SetRotation(uint8_t rotation) {
    ILI9341_WriteCommand(ILI9341_MADCTL);
    switch(rotation) {
        case 0:
            ILI9341_WriteData(0x88);  // Changed from 0x48
            break;
        case 1:
            ILI9341_WriteData(0x68);  // Changed from 0x28
            break;
        case 2:
            ILI9341_WriteData(0x48);  // Changed from 0x88
            break;
        case 3:
            ILI9341_WriteData(0xA8);  // Changed from 0xE8
            break;
    }
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if(x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
    if(x + w > ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if(y + h > ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;
    
    ILI9341_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    
    LCD_DC_HIGH();
    LCD_CS_LOW();
    
    uint8_t colorHigh = color >> 8;
    uint8_t colorLow = color & 0xFF;
    uint8_t buffer[2] = {colorHigh, colorLow};
    
    for(uint32_t i = 0; i < (uint32_t)w * h; i++) {
        HAL_SPI_Transmit(&hspi5, buffer, 2, HAL_MAX_DELAY);
    }
    
    LCD_CS_HIGH();
}

void ILI9341_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t x = r;
    int16_t y = 0;
    int16_t radiusError = 1 - x;
    
    while(x >= y) {
        // Draw horizontal lines to fill the circle
        for(int16_t i = x0 - x; i <= x0 + x; i++) {
            ILI9341_DrawPixel(i, y0 + y, color);
            ILI9341_DrawPixel(i, y0 - y, color);
        }
        for(int16_t i = x0 - y; i <= x0 + y; i++) {
            ILI9341_DrawPixel(i, y0 + x, color);
            ILI9341_DrawPixel(i, y0 - x, color);
        }
        
        y++;
        if(radiusError < 0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

void ILI9341_DrawChar(uint16_t x, uint16_t y, char ch, FontDef_t* font, uint16_t color, uint16_t bgcolor) {
    if(x + font->FontWidth >= ILI9341_WIDTH || y + font->FontHeight >= ILI9341_HEIGHT) return;
    
    uint32_t charIndex = (ch - 32) * font->FontHeight;
    
    for(uint8_t row = 0; row < font->FontHeight; row++) {
        uint16_t line = font->data[charIndex + row];
        
        for(uint8_t col = 0; col < font->FontWidth; col++) {
            if(line & (1 << (15 - col))) {
                ILI9341_DrawPixel(x + col, y + row, color);
            } else {
                ILI9341_DrawPixel(x + col, y + row, bgcolor);
            }
        }
    }
}

void ILI9341_DrawText(uint16_t x, uint16_t y, const char* str, FontDef_t* font, uint16_t color, uint16_t bgcolor) {
    uint16_t cursorX = x;
    uint16_t cursorY = y;
    
    while(*str) {
        if(*str == '\n') {
            cursorY += font->FontHeight;
            cursorX = x;
        } else if(*str == '\r') {
        } else {
            if(cursorX + font->FontWidth >= ILI9341_WIDTH) {
                cursorY += font->FontHeight;
                cursorX = x;
            }
            if(cursorY + font->FontHeight >= ILI9341_HEIGHT) {
                break;
            }
            ILI9341_DrawChar(cursorX, cursorY, *str, font, color, bgcolor);
            cursorX += font->FontWidth;
        }
        str++;
    }
}

void ILI9341_DrawImage(uint16_t x, uint16_t y, Image* img) {
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + img->width - 1) >= ILI9341_WIDTH) return;
    if((y + img->height - 1) >= ILI9341_HEIGHT) return;
    
    ILI9341_SetAddressWindow(x, y, x + img->width - 1, y + img->height - 1);
    
    ILI9341_WriteCommand(ILI9341_RAMWR);
    
    LCD_DC_HIGH();
    LCD_CS_LOW();
    
    // Send pixel data
    uint8_t buffer[2];
    for(uint32_t i = 0; i < img->width * img->height; i++) {
        uint16_t pixel = img->data[i];
        buffer[0] = pixel >> 8;
        buffer[1] = pixel & 0xFF;
        HAL_SPI_Transmit(&hspi5, buffer, 2, HAL_MAX_DELAY);
    }
    
    LCD_CS_HIGH();
}