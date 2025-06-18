#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define LCD_ADDR 0x27

// ---------------- Delay Functions ----------------
void timer_init() {
    // Configure Timer0 for CTC mode
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64
    OCR0A = 249; // For 1ms @16MHz
}

void timer_delay_ms(uint16_t ms) {
    uint16_t i;
    for (i = 0; i < ms; i++) {
        TCNT0 = 0;
        TIFR0 |= (1 << OCF0A);
        while (!(TIFR0 & (1 << OCF0A)));
    }
}

void timer_delay_us(uint16_t us) {
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01); // Prescaler 8
    OCR0A = (F_CPU / 8 / 1000000) - 1;

    uint16_t i;
    for (i = 0; i < us; i++) {
        TCNT0 = 0;
        TIFR0 |= (1 << OCF0A);
        while (!(TIFR0 & (1 << OCF0A)));
    }

    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64
    OCR0A = 249;
}


// ---------------- I2C & LCD Functions ----------------
void TWI_init(void) {
    TWSR = 0x00;
    TWBR = 0x48;
    TWCR = (1 << TWEN);
}

void TWI_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void TWI_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    timer_delay_us(10);
}

void TWI_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void LCD_send_nibble(uint8_t nibble, uint8_t control) {
    uint8_t data = nibble | control | 0x08;
    TWI_start();
    TWI_write(LCD_ADDR << 1);
    TWI_write(data);
    TWI_write(data | 0x04);
    timer_delay_us(1);
    TWI_write(data & ~0x04);
    TWI_stop();
    timer_delay_us(50);
}

void LCD_send_byte(uint8_t byte, uint8_t control) {
    LCD_send_nibble(byte & 0xF0, control);
    LCD_send_nibble((byte << 4) & 0xF0, control);
}

void LCD_command(uint8_t cmd) {
    LCD_send_byte(cmd, 0x00);
    if (cmd == 0x01 || cmd == 0x02)
        timer_delay_ms(2);
}

void LCD_data(uint8_t data) {
    LCD_send_byte(data, 0x01);
}

void LCD_init(void) {
    timer_delay_ms(50);
    LCD_send_nibble(0x30, 0x00); timer_delay_ms(5);
    LCD_send_nibble(0x30, 0x00); timer_delay_us(150);
    LCD_send_nibble(0x30, 0x00); timer_delay_us(150);
    LCD_send_nibble(0x20, 0x00);
    LCD_command(0x28);
    LCD_command(0x0C);
    LCD_command(0x06);
    LCD_command(0x01);
    timer_delay_ms(2);
}

void LCD_print(const char *str) {
    while (*str) LCD_data(*str++);
}

void LCD_set_cursor(uint8_t row, uint8_t col) {
    uint8_t pos = (row == 0 ? 0x00 : 0x40) + col;
    LCD_command(0x80 | pos);
}

// ---------------- Keypad Functions ----------------
char keypad_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

void keypad_init() {
    // Original columns (PC0-PC3) now act as rows - set as OUTPUT
    DDRC |= 0b00001111;    // PC0-PC3 as outputs (new rows)
    PORTC |= 0b00001111;   // Set all new rows HIGH
    
    // Original rows (PD2-PD5) now act as columns - set as INPUT with pull-up
    DDRD &= ~0b00111100;   // PD2-PD5 as inputs (new columns)
    PORTD |= 0b00111100;   // Enable pull-ups on new columns
}

char keypad_getkey() {
    uint8_t row, col;
    
    // Scan new rows (original columns)
    for (row = 0; row < 4; row++) {
        // Set all new rows HIGH then pull current new row LOW
        PORTC |= 0b00001111;
        PORTC &= ~(1 << row);
        
        // Check each new column (original rows)
        for (col = 0; col < 4; col++) {
            if (!(PIND & (1 << (col + 2)))) {  // Check PD2-PD5
                timer_delay_ms(20);            // Debounce
                if (!(PIND & (1 << (col + 2)))) {
                    while (!(PIND & (1 << (col + 2)))); // Wait for release
                    timer_delay_ms(20);
                    return keypad_map[row][col];
                }
            }
        }
    }
    return 0;
}
// ---------------- Load Control Functions ----------------
void load_init() {
    DDRB |= 0x0F;     // PB0–PB3 as output
    PORTB |= ~(0x06);
}


void set_load(uint8_t index, uint8_t on) {
    // PB0 and PB3 are active-high, PB1 and PB2 are active-low
    if (index == 1 || index == 2) {
        // Invert logic for PB1 and PB2
        if (on)
            PORTB |= (1 << index);   // ON (active-low)
        else
            PORTB &= ~(1 << index);  // OFF (active-low)
    } else {
        // Normal logic for PB0 and PB3
        if (on)
            PORTB |= (1 << index);   // ON (active-high)
        else
            PORTB &= ~(1 << index);  // OFF (active-high)
    }
}

void update_lcd_status() {
    LCD_command(0x01);
    LCD_set_cursor(0, 0);

    LCD_print("L1=");
    LCD_print((PORTB & (1 << 0)) ? "OFF " : "ON");
    LCD_print(" L2=");
    LCD_print((PORTB & (1 << 1)) ? "ON" : "OFF");

    LCD_set_cursor(1, 0);
    LCD_print("L3=");
    LCD_print((PORTB & (1 << 2)) ? "ON" : "OFF");
    LCD_print(" L4=");
    LCD_print((PORTB & (1 << 3)) ? "OFF" : "ON");
}

// ---------------- Main ----------------
int main(void) {
    timer_init();
    TWI_init();
    LCD_init();
    keypad_init();
    load_init();

    char password[5] = "1234";
    char input[5] = {0};
    uint8_t idx, i;
    char key;

    while (1) {
        LCD_command(0x01);
        LCD_print("Enter Password:");
        idx = 0;
        memset(input, 0, sizeof(input));

        while (idx < 4) {
            key = keypad_getkey();
            if (key) {
                LCD_set_cursor(1, idx);
                LCD_data('*');
                input[idx++] = key;
            }
        }

        input[4] = '\0';

 if (strncmp(input, password, 4) == 0) {
    LCD_command(0x01);
    LCD_print("Access Granted");
    timer_delay_ms(2000);
    update_lcd_status();

    while (1) {
        key = keypad_getkey();
        if (!key) continue;

        switch (key) {
            case 'A':  // Turn ON all loads (with correct logic for active-low/high)
                set_load(0, 0);  // PB0 active-high (ON)
                set_load(1, 1);  // PB1 active-low (ON = 0)
                set_load(2, 1);  // PB2 active-low (ON = 0)
                set_load(3, 0);  // PB3 active-high (ON)
                break;
                
            case 'B':  // Turn OFF all loads
                set_load(0, 1);  // PB0 active-high (OFF)
                set_load(1, 0);  // PB1 active-low (OFF = 1)
                set_load(2, 0);  // PB2 active-low (OFF = 1)
                set_load(3, 1);  // PB3 active-high (OFF)
                break;
                
            case '1': set_load(0, 0); break;  // Toggle PB0 (active-high)
            case '2': set_load(0, 1); break;
            case '3': set_load(1, 1); break;  // Toggle PB1 (active-low)
            case '4': set_load(1, 0); break;
            case '5': set_load(2, 1); break;  // Toggle PB2 (active-low)
            case '6': set_load(2, 0); break;
            case '7': set_load(3, 0); break;  // Toggle PB3 (active-high)
            case '8': set_load(3, 1); break;
            case 'D': goto reset;
        }
        update_lcd_status();
    }
        } else {
            LCD_command(0xC0);
            LCD_print("Wrong Password");
            timer_delay_ms(2000);
        }

    reset: 
        continue;
    }
}
