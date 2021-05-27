//####################### BUTTON VALUES ##########################

#define BTN P2_5            // Button pin
#define BTN_LED P2_4        // Button LED for visual indication
#define BTN_LED_ENABLE true      // Enable/disable the button led

#define BTN_LED_CHECK 

bool btnLedsInUse = false;

unsigned long btnPressLength = 0;
bool btnPressed = false;

//####################### BUTTON MODE CONFIG #####################

#define BTN_MAX_HOLD_LENGTH_MICROS 600000000 // 10 minutes maximum hold

#define MODE_PARTY_MINIMUM_MICROS 30000
#define MODE_PARTY_ROUNDS       5
#define MODE_PARTY_DELAY_MS     75
#define MODE_PARTY_ROUND_DELAY_MS 50

#define MIN_MICROS 0
#define MAX_MICROS 1
#define MODE_LED_PIN        2

#define MODELEN    3

#define SPEED_MODE 1
#define BREATH_TYPE_MODE 2

long btnLedModes[][3] = {
  { 0, 1000000, P2_4 },
  { 100000, 2000000, P2_6 },
  { 2000000, 4000000, P2_7 }
};

//####################### LED VALUES ##############################

#define LEDLEN                  2            // Number of LEDs

// LED Configuration Table
// Default values are invalid until initialized and marked as updating
int ledConfigs[][9] = {
  {0,0,0,0,0, P1_6, 0, 1, 1}, 
  {0,0,0,0,0, P2_1, 0, 1, 1}
};

#define BREATH_HOLD_PERCENT     0
#define BREATH_HOLD_VALUE       1
#define BREATH_EMPTY_PERCENT    2
#define BREATH_EMPTY_VALUE      3
#define COUNT_MAX               4
#define LED_PIN                 5
// Dynamic Values
#define COUNTER                 6
#define DIRECTION               7
#define UPDATING                8

#define TRANSITION_LOW          0
#define TRANSITION_SMOOTH       1  
#define TRANSITION_HIGH         2

#define LED_TIMESTEP_MS         10
#define LED_COUNTER_STEP        2

//####################### WELCOME FLASH ############################

#define WELCOME_FLASH_ENABLED   true
#define WELCOME_FLASH_PULSE_MS  250
#define WELCOME_FLASH_PULSES    4

//####################### START CODE ###############################

void setup() {
  //Set up the LEDs
  for(int i=0;i<LEDLEN;i++) {
    pinMode(ledConfigs[i][LED_PIN], OUTPUT);
    // Initialize Counters
    ledConfigs[i][COUNTER] = 0;    
    configureLeds(i, TRANSITION_HIGH);
  }
  
  // Serial init - temporary
  Serial.begin(9600);
  Serial.println("Hello!");

  // Enable button LED
  if(BTN_LED_ENABLE) {
    Serial.println("C");
    for(int i=0;i<MODELEN;i++) {
      pinMode(btnLedModes[i][MODE_LED_PIN], OUTPUT);
      digitalWrite(btnLedModes[i][MODE_LED_PIN], LOW);      
    }
  }

  // Flash Welcome
  welcomeFlash();

  // Set up the button as the last thing we do
  pinMode(BTN, INPUT_PULLUP);
  delay(15);
  attachInterrupt(digitalPinToInterrupt(BTN), btnPress, CHANGE); 
}

void configureLeds(unsigned int led, int transition) {
  // Take lock
  ledConfigs[led][UPDATING] = 1;
  
  // Save old params
  int oldCountMax = ledConfigs[led][COUNT_MAX];

  // Load new params
  ledConfigs[led][COUNT_MAX] = 450;
  ledConfigs[led][BREATH_HOLD_PERCENT] = 2;
  ledConfigs[led][BREATH_EMPTY_PERCENT] = 40;

  //dynamic values;
  if(transition == TRANSITION_HIGH) {
    ledConfigs[led][COUNTER] = ledConfigs[led][COUNT_MAX];
    ledConfigs[led][DIRECTION] = -1;
  } else if(transition == TRANSITION_SMOOTH && oldCountMax > 0) {
    ledConfigs[led][COUNTER] = ((float)ledConfigs[led][COUNTER]/oldCountMax)*(float)ledConfigs[led][COUNT_MAX];
  } else { // default is TRANSITION_LOW
    ledConfigs[led][COUNTER] = 0;
    ledConfigs[led][DIRECTION] = 1;
  }

  ledConfigs[led][BREATH_HOLD_VALUE] = ((float)ledConfigs[led][COUNT_MAX]*((100-ledConfigs[led][BREATH_HOLD_PERCENT])))/100;
  ledConfigs[led][BREATH_EMPTY_VALUE] = ((float)ledConfigs[led][COUNT_MAX]*(ledConfigs[led][BREATH_EMPTY_PERCENT]))/100;

  // Release lock
  ledConfigs[led][UPDATING] = 0;
}

void welcomeFlash() {  
  if(WELCOME_FLASH_ENABLED) {
    for(int i=0;i<LEDLEN;i++) {
      if(ledConfigs[i][UPDATING] == 1)
        return;
      ledConfigs[i][UPDATING] = 1;      
    }
    
    for(int i=0;i<WELCOME_FLASH_PULSES;i++) {
      for(int j=0;j<LEDLEN;j++) 
        digitalWrite(ledConfigs[j][LED_PIN], HIGH);
      delay(WELCOME_FLASH_PULSE_MS);
      for(int j=0;j<LEDLEN;j++) 
        digitalWrite(ledConfigs[j][LED_PIN], LOW);
      delay(WELCOME_FLASH_PULSE_MS);      
    }

    for(int i=0;i<LEDLEN;i++)
      ledConfigs[i][UPDATING] = 0;
  }
}

void btnLights() {
  if(!btnLedsInUse) {
    if(btnPressed) {
      unsigned long pLength = micros() - btnPressLength;
  
      for(int i=0;i<MODELEN+1;i++) {
        if(i==MODELEN) {
          for(int j=0;j<MODELEN;j++)
            digitalWrite(btnLedModes[j][MODE_LED_PIN], HIGH);
        }
        if(pLength > btnLedModes[i][MIN_MICROS] && pLength <= btnLedModes[i][MAX_MICROS]) {
          for(int j=0;j<MODELEN;j++)
            digitalWrite(btnLedModes[j][MODE_LED_PIN], i==j);
          break;
        }
      }
    } else {
      for(int j=0;j<MODELEN;j++)
        digitalWrite(btnLedModes[j][MODE_LED_PIN], LOW);    
    }    
  }
}

void btnPress() {   
   if(!btnPressed) {
    // Button is being pressed    

     btnPressed = true;
     btnPressLength = micros();

     Serial.println("B");    
   } else {
    // Button is being released
    
     btnPressed = false;
     btnPressLength = micros() - btnPressLength;

     Serial.println("D");
     Serial.println(btnPressLength);

     newMode(btnPressLength);
   }
}

void newMode(unsigned long buttonPressLength) {
    if(buttonPressLength > btnLedModes[MODELEN-1][MAX_MICROS])
      return;

    if(buttonPressLength < MODE_PARTY_MINIMUM_MICROS) {
      partyMode();
      return;
    }

   for(int i=1;i<MODELEN;i++) {
      if(buttonPressLength > btnLedModes[i][MIN_MICROS] && buttonPressLength <= btnLedModes[i][MAX_MICROS]) {
        Serial.println(i);
        break;
      }
   }
}

void partyMode() {
    for(int i=0;i<LEDLEN;i++) {
      if(ledConfigs[i][UPDATING] == 1)
        return;
      ledConfigs[i][UPDATING] = 1;      
    }

    for(int round=0;round<MODE_PARTY_ROUNDS; round++) {
      for(int led=0;led<LEDLEN+MODELEN;led++) {
        if(led >= LEDLEN) {
          btnLedsInUse = true;
          digitalWrite(btnLedModes[led-LEDLEN][MODE_LED_PIN],HIGH);
          delay(MODE_PARTY_DELAY_MS);
          digitalWrite(btnLedModes[led-LEDLEN][MODE_LED_PIN],LOW);
          btnLedsInUse = false;
        } else {
          digitalWrite(ledConfigs[led][LED_PIN],HIGH);
          delay(MODE_PARTY_DELAY_MS);
          digitalWrite(ledConfigs[led][LED_PIN],LOW);
        }
      }
      delay(MODE_PARTY_ROUND_DELAY_MS);
    }

    for(int i=0;i<LEDLEN;i++)
      ledConfigs[i][UPDATING] = 0;
}

void setBrightness(unsigned int led) {
  if(ledConfigs[led][UPDATING] == 1)
    return;
  
  int brightness = 0;

  if(ledConfigs[led][COUNTER] > ledConfigs[led][BREATH_HOLD_VALUE]) {
     brightness = 255; 
  } else if(ledConfigs[led][COUNTER] < ledConfigs[led][BREATH_EMPTY_VALUE]) {
    brightness = 0;
  } else {
    float pos = ((float)ledConfigs[led][COUNTER]-ledConfigs[led][BREATH_EMPTY_VALUE])/((float)ledConfigs[led][BREATH_HOLD_VALUE]-ledConfigs[led][BREATH_EMPTY_VALUE]);
    pos = (tanh((5*(pos-0.5)))/2)+0.5;
    brightness = pos*255;
  }

  analogWrite(ledConfigs[led][LED_PIN], brightness);
}

void stepLed(unsigned int led) {
  ledConfigs[led][COUNTER] += ledConfigs[led][DIRECTION]*LED_COUNTER_STEP;

  if(ledConfigs[led][COUNTER] >= ledConfigs[led][COUNT_MAX]) {
    ledConfigs[led][COUNTER] = ledConfigs[led][COUNT_MAX];
    ledConfigs[led][DIRECTION] = -1;
  } else if(ledConfigs[led][COUNTER] <= 0) {
    ledConfigs[led][COUNTER] = 0;
    ledConfigs[led][DIRECTION] = 1;
  }

  setBrightness(led);
}

void stepLeds() {
  for(int i=0;i<LEDLEN;i++)
    stepLed(i);
}

void loop() {  
  stepLeds();
  btnLights();
  delay(LED_TIMESTEP_MS);
}
