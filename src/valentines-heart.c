/**
 * Author: Michael Rosa
 * Date: January 2018
 *
 * Uses an ATtiny85 to drive a charlieplexed matrix of LEDs 
 * in the shape of a heart with different patterns
 *
 * After about a minute, the AVR shuts itself off through the use
 * of a separate analog circuit. In case this circuit fails the AVR 
 * also goes into low power mode.
 */
 
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <string.h>

#define LINE0 PB0
#define LINE1 PB1
#define LINE2 PB4
#define LINE3 PB3
#define ALL_LINES 1 << LINE0 | 1 << LINE1 | 1 << LINE2 | 1 << LINE3

#define POWER PB2

#define LED_DELAY 800

#define CHASE_DELAY 2
#define BEAT_DELAY  20


bool leds[10] = {false, false, false, false, false,
                 false, false, false, false, false};
                 
void reset_leds(void) {
  DDRB  &= ~(ALL_LINES);
  PORTB &= ~(ALL_LINES);
}
                 
void drive_leds(uint8_t line0, uint8_t line1, bool b0, bool b1) {
  if(line0 == line1 | 
     !((1 << line0) & ALL_LINES) |
     !((1 << line1) & ALL_LINES) |
     !(b0 | b1)) {
    return;
  }
  
  reset_leds();
  DDRB |= 1 << line0 | 1 << line1;
  
  if(b0) {
    PORTB |= 1 << line0;
    _delay_us(LED_DELAY);
    PORTB &= ~(1 << line0);
  }
  if(b1) {
    PORTB |= 1 << line1;
    _delay_us(LED_DELAY);
    PORTB &= ~(1 << line1);
  }
  
  reset_leds();
}

ISR(TIM0_OVF_vect) {
  drive_leds(LINE0, LINE1, leds[0], leds[1]);
  drive_leds(LINE0, LINE2, leds[2], leds[3]);
  drive_leds(LINE1, LINE2, leds[4], leds[5]);
  drive_leds(LINE1, LINE3, leds[6], leds[7]);
  drive_leds(LINE2, LINE3, leds[8], leds[9]);
}

void chase_pattern(void) {
  memset(leds, false, sizeof(leds));
  uint8_t times = 2;
  while(times-- > 0) {
    leds[0] = true;
    leds[5] = true;
    sei(); 
    for(uint8_t i = 6; i <= sizeof(leds); i++) {
      _delay_ms(CHASE_DELAY);
      leds[i - 6] = false;
      leds[i - 1] = false;
      if(i < sizeof(leds)){
        leds[i - 5] = true;
        leds[i] = true;
      }
    }
  }
  cli();
}

void beat_pattern(void) {
  memset(leds, true, sizeof(leds));
  sei();
  _delay_us(BEAT_DELAY);
  cli();
  _delay_ms(100);
  sei();
  _delay_us(BEAT_DELAY);
  cli();
}

int main(void)
{
  DDRB |= 1 << POWER;
  
  TCCR0B |= 1 << CS01;  // prescaler 8
  TIMSK |= 1 << TOIE0;  // enable interrupts
  TCNT0 = 0;            // about every 2 ms
  sei();
  
  while(1)
  {
    chase_pattern();
    for(int i = 0; i < 3; i++) {
      _delay_ms(500);
      beat_pattern();
    }
    _delay_ms(500);
  }
  return 0;
} 