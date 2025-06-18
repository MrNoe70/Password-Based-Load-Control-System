# Password-Based Load Control System

## Team Members
- NOE S SETENTA JR
- JAH ISAAC CAGULA

---

## Project Description
This project implements a password-based load control system using an AVR microcontroller (Arduino Uno). The system authenticates users via a 4x4 keypad. Upon entering the correct password, the authorized user can control four loads represented by LEDs. Users can selectively turn ON or OFF any combination of the four loads using additional commands entered through the keypad.

---

## Key Features
- Password authentication to restrict access
- Control of four independent loads
- User-friendly interface with 16x2 I2C LCD for feedback

---

## System Specifications

| Feature          | Description                      |
|------------------|--------------------------------|
| **Input Device** | 4x4 Matrix Keypad               |
| **Output Devices**| 16x2 I2C LCD, Relay Module (4 loads) |
| **Microcontroller** | Arduino Uno (16 MHz)           |

---

## How It Works

1. User inputs a password using the 4x4 keypad.
2. The system verifies the password.
3. Upon successful authentication, the user can send commands via the keypad to turn ON or OFF any of the four connected loads.
4. The 16x2 LCD displays status messages and feedback for user actions.

---

## Usage

- Enter password to gain control access.
- Use keypad commands to toggle the states of the four loads.
- Refer to the project documentation for detailed command mappings.

---

## Hardware Components

- Arduino Uno (ATmega328P)
- 4x4 Matrix Keypad
- 16x2 I2C LCD Display
- 4-Channel Relay Module (for load control)
- Connecting wires and power supply

---

## Software

- Written in C using AVR-GCC (bare-metal programming)
- Interfaces with keypad, LCD, and relay module using AVR registers

---

## License

This project is for educational purposes.

---

## Contact

For questions or feedback, please reach out to the team members listed above.

