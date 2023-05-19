#define LED_13 (1<<5)
#define LED_12 (1<<4)
#define LED_11 (1<<3)
#define LED_10 (1<<2)

#define ALL_LEDS (LED_10|LED_11|LED_12|LED_13)

#define BUTTON_A1 (1<<1)
#define BUTTON_A2 (1<<2)
#define BUTTON_A3 (1<<3)

enum modes {BLINK_INCR, BLINK_DECR, BLINK_ALL, DO_NOTHING} mode = DO_NOTHING;

volatile uint32_t sysTick = 0;
// The interruput is triggered every millisecond
ISR(TIMER1_COMPA_vect) {
  sysTick++;
}

uint32_t getSysTick(void) {
  return sysTick;
}

void A1_buttonHandler(void) {
  static uint8_t s0=0, s1=0, s2=0;
  static uint8_t prev_state = 0;

  s2 = s1;
  s1 = s0;
  s0 = (PINC & BUTTON_A1) == 0;

  // Change state if three consecutive readings return the same value (LOW or HIGH)
  uint8_t curr_state = prev_state & (s0|s1|s2) | (s0&s1&s2);

  // Change mode if raising edge is detected
  if ((curr_state==1) && (prev_state==0))
    mode = BLINK_INCR;
    
  prev_state = curr_state;
}

void A2_buttonHandler(void) {
  static uint8_t s0=0, s1=0, s2=0;
  static uint8_t prev_state = 0;

  s2 = s1;
  s1 = s0;
  s0 = (PINC & BUTTON_A2) == 0;

  // Change state if three consecutive readings return the same value
  uint8_t curr_state = prev_state & (s0|s1|s2) | (s0&s1&s2);

  // Change mode if raising edge is detected
  if ((curr_state==1) && (prev_state==0))
    mode = BLINK_DECR;
  
  prev_state = curr_state;
}

void A3_buttonHandler(void) {
  static uint8_t s0=0, s1=0, s2=0;
  static uint8_t prev_state = 0;

  static uint32_t local_time = 0;
  static bool pressed = false;

  s2 = s1;
  s1 = s0;
  s0 = (PINC & BUTTON_A3) == 0;

  // Change state if three consecutive readings return the same value
  uint8_t curr_state = prev_state & (s0|s1|s2) | (s0&s1&s2);

  // Change mode if raising edge is detected
  if ((curr_state==1) && (prev_state==0)) {
    // Turn all leds off
    PORTB  = ALL_LEDS;
    mode = BLINK_ALL;

    pressed = true;
    local_time = getSysTick();
  }

  // Change state if button A3 is held for more than 2 seconds
  if(pressed) {
    if ( (PINC & BUTTON_A3) != 0)
      pressed = false;
    
    else if (getSysTick()-local_time > 2000) {
      pressed = false;
      mode = DO_NOTHING;
    }
  }
  
  prev_state = curr_state;
}

int main(void) {

  // Configure pins
  DDRB = ALL_LEDS;       // Sets pins 10 to 13 to output

  // Configuring timer interrupts
  TCCR1A = 0b00000000;
  TCCR1B = 0b00001100; // Sets prescaler to 256 and mode to CTC
  OCR1A  = 62-1;
  TIMSK1 = 0b00000010; // Enables output compare match interrupt A
  sei();               // Enables interrupts;

  uint8_t leds[] = {LED_10, LED_11, LED_12, LED_13};
  size_t index = 0;

  PORTC |= BUTTON_A1 | BUTTON_A2 | BUTTON_A3;

  uint32_t scan_time = getSysTick();
  uint32_t update_time = getSysTick();
  while(true) {
    if (getSysTick() >= scan_time) {
      scan_time += 15;
      
      A1_buttonHandler();
      A2_buttonHandler();
      A3_buttonHandler();

      switch(mode) {
          
        case BLINK_INCR:
          if (getSysTick()-update_time > 200) {
            update_time = getSysTick();
            PORTB = ~leds[index++];
            index %= 4;
          }
          break;
        
        case BLINK_DECR:
          if (getSysTick()-update_time > 200) {
            update_time = getSysTick();
            PORTB = ~leds[index--];
            index %= 4;
          }
          break;
        
        case BLINK_ALL:
          if (getSysTick()-update_time > 140) {
            update_time = getSysTick();
            PORTB ^= ALL_LEDS;
          }
          break;
        
        case DO_NOTHING:
          PORTB = ALL_LEDS;
          break;
      }
    }
  }
  
  return 0;
}
