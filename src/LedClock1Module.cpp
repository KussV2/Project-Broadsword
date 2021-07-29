#include "LedClock1Module.h"
#include "Events.h"

/* Public */

LedClock1Module::LedClock1Module() {
  // Setup 7-segment indicator rows pins
  // Segment is enabled by LOW (inverse) level
  pinMode( SEGMENT_A_PIN, OUTPUT );
  pinMode( SEGMENT_B_PIN, OUTPUT );
  pinMode( SEGMENT_C_PIN, OUTPUT );
  pinMode( SEGMENT_D_PIN, OUTPUT );
  pinMode( SEGMENT_E_PIN, OUTPUT );
  pinMode( SEGMENT_F_PIN, OUTPUT );
  pinMode( SEGMENT_G_PIN, OUTPUT );
  pinMode( SEGMENT_S_PIN, OUTPUT );

  // Setup 7-segment indicator columns pins
  pinMode( DIGIT_1_PIN, OUTPUT );
  pinMode( DIGIT_2_PIN, OUTPUT );
  pinMode( DIGIT_3_PIN, OUTPUT );
  pinMode( DIGIT_4_PIN, OUTPUT );

  clearDisplay();

  displayBuffer[0] = DASH_START;
  displayBuffer[1] = DASH_START;
  displayBuffer[2] = DASH_START;
  displayBuffer[3] = DASH_START;
  showDots = true;

  ticker.attach_ms( REFRESH_RATE, ticker_callback, this );

  eventBusToken = Bus.listen<StatusChangedEvent>( [this](const StatusChangedEvent& event) {
    if( strcmp( event.module->getId(), RTC_MODULE ) == 0 ) {      
      String datetime = event.payload;
      int i = datetime.indexOf( 'T' );
      displayBuffer[0] = digitToSymbol(datetime.charAt(i + 1));
      displayBuffer[1] = digitToSymbol(datetime.charAt(i + 2));
      displayBuffer[2] = digitToSymbol(datetime.charAt(i + 4));
      displayBuffer[3] = digitToSymbol(datetime.charAt(i + 5));
    }
  });
}

char LedClock1Module::digitToSymbol( unsigned char digit )
{
  if( digit >= '1' && digit <= '9' ) 
  {
    return digit - 48;
  }
  else if(digit == '-')
  {
    return 10;
  }
  else
  {
    return 11;
  }
};


LedClock1Module::~LedClock1Module() {
  Bus.unlisten<StatusChangedEvent>( eventBusToken );
  ticker.detach();
}

/* Private */

void LedClock1Module::clearDisplay() {
  digitalWrite( DIGIT_1_PIN, HIGH );
  digitalWrite( DIGIT_2_PIN, HIGH );
  digitalWrite( DIGIT_3_PIN, HIGH );
  digitalWrite( DIGIT_4_PIN, HIGH );
}

void LedClock1Module::showNextSymbol() {
  showSymbol( displayBuffer[currentPosition], currentPosition );

  if( ++currentPosition > 3 ) {
    currentPosition = 0;
  }
}

void LedClock1Module::showSymbol( uint8_t symbol, uint8_t position ) {
  // The used pcb doesn't have a led on the F segment.
  // Practically it means it could display only digits 1, 2, 3 and 7
  if( position == 0 ) {
    switch( symbol ) {
      case 1:
      case 2:
        break;
      default:
        symbol = DIGIT_OFF;
    }
  }

  uint8_t bitset = SYMBOL_TO_BITSET[ symbol ];

  if( (position == 1 && showAlarm) ||
      (position == 2 && showDots) ) {
    bitset &= 0b11111110;
  }

  uint8_t i = 0;
  uint8_t mask = 0b10000000;
  do {
    bool bit = bitset & mask;
    digitalWrite( BIT_TO_PIN[i], bit );
    mask = mask >> 1;
  } while( ++i < 8 );

  for( i = 0; i < 4; i++ ) {
    digitalWrite( DIGIT_TO_PIN[i], position == i ? LOW : HIGH );
  }
}