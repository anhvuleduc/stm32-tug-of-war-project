#ifndef ILI9341_H
#define ILI9341_H

#include "main.h"
#include "fonts.h"
#include "img.h"

// ILI9341 dimensions
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320

// Color definitions (RGB565)
#define ILI9341_BLACK   0x0000
#define ILI9341_BLUE    0x001F
#define ILI9341_RED     0xF800
#define ILI9341_GREEN   0x07E0
#define ILI9341_CYAN    0x07FF
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW  0xFFE0
#define ILI9341_WHITE   0xFFFF

// Pin definitions - adjust these to match your hardware
#define LCD_CS_PIN   GPIO_PIN_2   // PC2
#define LCD_CS_PORT  GPIOC
#define LCD_DC_PIN   GPIO_PIN_13  // PD13
#define LCD_DC_PORT  GPIOD
#define LCD_RST_PIN  GPIO_PIN_12  // PD12
#define LCD_RST_PORT GPIOD

// Macros for pin control
#define LCD_CS_LOW()   HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define LCD_CS_HIGH()  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define LCD_DC_LOW()   HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define LCD_DC_HIGH()  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET)
#define LCD_RST_LOW()  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET)
#define LCD_RST_HIGH() HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET)

// Function prototypes
void ILI9341_Init(void);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_SetRotation(uint8_t rotation);
void ILI9341_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void ILI9341_DrawChar(uint16_t x, uint16_t y, char ch, FontDef_t* font, uint16_t color, uint16_t bgcolor);
void ILI9341_DrawText(uint16_t x, uint16_t y, const char* str, FontDef_t* font, uint16_t color, uint16_t bgcolor);
void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_DrawImage(uint16_t x, uint16_t y, Image* img);

#endif // ILI9341_H














