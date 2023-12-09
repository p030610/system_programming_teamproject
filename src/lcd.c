#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdio.h>

#define I2C_ADDR 0x27
#define LCD_WIDTH 16

#define LCD_CHR 1
#define LCD_CMD 0

#define LCD_LINE_1 0x80
#define LCD_LINE_2 0xC0
#define LCD_LINE_3 0x94
#define LCD_LINE_4 0xD4

#define LCD_BACKLIGHT 0x08
#define ENABLE 0b00000100

#define E_PULSE 0.0005
#define E_DELAY 0.0005

int fd;

void lcd_init();
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);
void lcd_string(const char *message, int line);

int main()
{
    wiringPiSetup();
    fd = wiringPiI2CSetup(I2C_ADDR);

    lcd_init();

    while (1)
    {
        lcd_string("10      ", LCD_LINE_1);
        lcd_string("20      ", LCD_LINE_2);
        delay(3000); // 3 seconds delay
    }

    return 0;
}

void lcd_init()
{
    lcd_byte(0x33, LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    lcd_byte(0x01, LCD_CMD);

    delay(E_DELAY * 1000);
}

void lcd_byte(int bits, int mode)
{
    int bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    int bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    wiringPiI2CWrite(fd, bits_high);
    lcd_toggle_enable(bits_high);

    wiringPiI2CWrite(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)
{
    delay(E_DELAY);
    wiringPiI2CWrite(fd, (bits | ENABLE));
    delay(E_PULSE);
    wiringPiI2CWrite(fd, (bits & ~ENABLE));
    delay(E_DELAY);
}

void lcd_string(const char *message, int line)
{
    lcd_byte(line, LCD_CMD);

    int i;
    for (i = 0; i < LCD_WIDTH; i++)
    {
        lcd_byte(message[i], LCD_CHR);
    }
}
