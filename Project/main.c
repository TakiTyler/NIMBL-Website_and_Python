#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <driverlib.h>

// using GPIO P1.0, P1.1, P1.2 for LCD
// pin declarations
#define LCD_PORT_OUT P1OUT
#define LCD_PORT_DIR P1DIR
#define CS_PIN BIT0
#define WR_PIN BIT1
#define DATA_PIN BIT2

// ht1621 commands
#define BIAS_CMD 0x29 // 4 commons option, 1/3 bias
#define SYS_EN 0x01
#define LCD_OFF 0x02 // 100 00000010 X
#define LCD_ON 0x03 // 100 00000011 X
#define CMD_MODE 0x04 // 0000 0100
#define WRITE_MODE 0x05 // 101 needed

#define LCD_CS BIT0
#define LCD_WR BIT1
#define LCD_DATA BIT2

// digit map
const uint8_t digit_map[] = {
    0xD7, 0x06, 0xE3, 0xA7, 0x36, 0xB5, 0xF5, 0x07, 0xF7, 0xB7
};

// sends MSB one-by-one
void send_bits(uint8_t val, uint8_t num_bits){
    uint8_t i;
    for(i = 0; i < num_bits; i++){
        // prepare write
        LCD_PORT_OUT &= ~WR_PIN;

        // check MSB
        if(val & 0x80){
            LCD_PORT_OUT |= DATA_PIN;
        }
        else{
            LCD_PORT_OUT &= ~DATA_PIN;
        }

        LCD_PORT_OUT |= WR_PIN;

        // bit shift left
        val <<= 1;
    }
}

// writin a command (header of 100 & last bit padding)
void write_command(uint8_t cmd){
    LCD_PORT_OUT &= ~CS_PIN; // send low, starts com
    send_bits(CMD_MODE << 5, 3); // sending header bits
    send_bits(cmd, 8); // sending command
    send_bits(0, 1); // padding bit
    LCD_PORT_OUT |= CS_PIN;
}

// write data to display memory
void write_memory(uint8_t address, uint8_t data){
    LCD_PORT_OUT &= ~CS_PIN; // send low, starts com
    send_bits(WRITE_MODE << 5, 3); // sending header bits
    send_bits(address << 2, 6); // sending 32-bit address
    send_bits(data << 4, 4); // send data
    LCD_PORT_OUT |= CS_PIN;
}

void LCD_init(){
    // configure pins
    LCD_PORT_DIR |= CS_PIN | WR_PIN | DATA_PIN;
    LCD_PORT_OUT |= CS_PIN | WR_PIN | DATA_PIN;

    write_command(SYS_EN);
    write_command(LCD_ON);
    write_command(BIAS_CMD);
}

// controls either the "heart rate segments" or "SpO2 segments"
void display_three_digits(bool heart_rate_segments, uint16_t num){
    
    if(num > 999) return;
    
    bool digit_written = false;
    uint16_t temp;
    uint8_t address;
    uint8_t digits[3];

    if(heart_rate_segments == true){
        // digits[] = {2, 1, 0}; // digits used for heart-rate
        digits[0] = 2;
        digits[1] = 1;
        digits[2] = 0;
    }
    else{
        // digits[] = {5, 4, 3}; // digits used for SpO2
        digits[0] = 5;
        digits[1] = 4;
        digits[2] = 3;
    }

    if(num / 100 != 0){
        // write hundreths place
        address = digits[0] * 2;
        write_memory(address, digit_map[num / 100] & 0x0F); // send lower 4 bits
        write_memory(address+1, (digit_map[num / 100] >> 4) & 0x0F); // send upper 4 bits
        // remove hundreths place
        temp = num / 100; // this should contain the hundreths place
        temp *= 100; // goes back to hundreds
        num = num - temp; // should remove the hundreths place
        digit_written = true;
    }
    else{
        address = digits[0] * 2;
        write_memory(address, 0x00 & 0x0F); // send lower 4 bits
        write_memory(address+1, (0x00 >> 4) & 0x0F); // send upper 4 bits
    }

    // need logic if number is something like 900
    if(num / 10 != 0){
        // write tenths place
        address = digits[1] * 2;
        write_memory(address, digit_map[num / 10] & 0x0F); // send lower 4 bits
        write_memory(address+1, (digit_map[num / 10] >> 4) & 0x0F); // send upper 4 bits
        // remove tenths place
        temp = num / 10; // this should contain the tenths place
        temp *= 10; // goes back to tens
        num = num - temp; // should remove the tenths place
        digit_written = true;
    }
    else if(num / 10 == 0 && digit_written == true){
        address = digits[1] * 2;
        write_memory(address, digit_map[0] & 0x0F); // send lower 4 bits
        write_memory(address+1, (digit_map[0] >> 4) & 0x0F); // send upper 4 bits
    }
    else{
        address = digits[1] * 2;
        write_memory(address, 0x00 & 0x0F); // send lower 4 bits
        write_memory(address+1, (0x00 >> 4) & 0x0F); // send upper 4 bits
    }

    if(num == 0){
        address = digits[2] * 2;
        write_memory(address, digit_map[0] & 0x0F); // send lower 4 bits
        write_memory(address+1, (digit_map[0] >> 4) & 0x0F); // send upper 4 bits
    }
    else{ // write non-zero digit
        address = digits[2] * 2;
        write_memory(address, digit_map[num] & 0x0F); // send lower 4 bits
        write_memory(address+1, (digit_map[num] >> 4) & 0x0F); // send upper 4 bits
    }

    // uint8_t address = position * 2;

    // write_memory(address, segments & 0x0F); // send lower 4 bits
    // write_memory(address+1, (segments >> 4) & 0x0F); // send upper 4 bits
}

void test_display(uint8_t position, uint8_t hex){

    // write_memory(address, hex & 0x0F); // send lower 4 bits
    // write_memory(address+1, (hex >> 4) & 0x0F); // send upper 4 bits
}

void clear_display(){
    uint8_t i;
    for(i = 0; i < 32; i++){
        write_memory(i, 0x00);
    }
}

int main(void) {

    volatile uint32_t i;
    volatile uint32_t j;
    volatile uint32_t increment = 0;
    volatile uint32_t decrement = 999;
    uint8_t lcd_segment_testing;

    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // Set P1.0 to output direction
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN0
    );

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    // enable GPIO for LCD
    P2DIR |= LCD_CS | LCD_DATA | LCD_WR;
    P2OUT |= LCD_CS;

    LCD_init();
    clear_display();

    // sections 2-0 are the right 3 digits
    // sections 

    display_three_digits(true, 400);
    display_three_digits(false, 623);

    while(1){    
        display_three_digits(true, increment);
        display_three_digits(false, decrement);
        increment++;
        decrement--;
        for(j=20000; j>0; j--);
        if(decrement == 0) break;
        // lcd_segment_testing = 0x00;
        // while(1){
        //     if(lcd_segment_testing == 0x80) break;

        //     test_display(i, lcd_segment_testing);
        //     if(lcd_segment_testing == 0x00) lcd_segment_testing++;
        //     else{
        //         lcd_segment_testing = lcd_segment_testing + lcd_segment_testing;
        //     }
        //     for(j=40000; j>0; j--);
        //     for(j=40000; j>0; j--);
        //     for(j=40000; j>0; j--);
        // }
        // test_display(i, 0xFF);
    }

    while(1)
    {
        // Toggle P1.0 output
        GPIO_toggleOutputOnPin(
            GPIO_PORT_P1,
            GPIO_PIN0
        );

        // Delay
        for(i=10000; i>0; i--);
    }
}
