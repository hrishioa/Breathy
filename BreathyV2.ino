// Written in Energia due to lack of time


//####################### BUTTON VALUES ##########################

#define BTN P2_5            // Button pin
#define BTN_LED P2_4        // Button LED for visual indication
#define BTN_LED_ENABLE true      // Enable/disable the button led

bool btnLedInUse = false;

unsigned long btnPressLength = 0;
bool btnPressed = false;

//####################### BUTTON MODE CONFIG #####################

#define BTN_MAX_HOLD_LENGTH_MICROS 600000000 // 10 minutes maximum hold

#define MODE_PARTY_MINIMUM_MICROS 30000
#define MODE_PARTY_ROUNDS       10
#define MODE_PARTY_DELAY_MS     75
#define MODE_PARTY_ROUND_DELAY_MS 50

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
  Serial.println("Hello");

  // Enable button LED
  if(BTN_LED_ENABLE) {
    pinMode(BTN_LED, OUTPUT);
    digitalWrite(BTN_LED, LOW);
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

void btnPress() {   
   if(!btnPressed) {
    // Button is being pressed    

    btnPressed = true;
    btnPressLength = micros();

    if(BTN_LED_ENABLE && !btnLedInUse)
      digitalWrite(BTN_LED, HIGH);

    Serial.println("P");    
   } else {
    // Button is being released

    btnPressed = false;
    btnPressLength = micros() - btnPressLength;

    // TODO: Once we know button press length, do something with it

    if(BTN_LED_ENABLE)
      digitalWrite(BTN_LED, LOW);

     Serial.println("D");
     Serial.println(btnPressLength);

     newMode(btnPressLength);
   }
}

void newMode(unsigned long buttonPressLength) {
    if(buttonPressLength > BTN_MAX_HOLD_LENGTH_MICROS)
      return;

    if(buttonPressLength < MODE_PARTY_MINIMUM_MICROS) {
      partyMode();
    }
}

void partyMode() {
    for(int i=0;i<LEDLEN;i++) {
      if(ledConfigs[i][UPDATING] == 1)
        return;
      ledConfigs[i][UPDATING] = 1;      
    }

    for(int round=0;round<MODE_PARTY_ROUNDS; round++) {
      for(int led=0;led<LEDLEN+1;led++) {
        if(led == LEDLEN) {
          btnLedInUse = true;
          digitalWrite(BTN_LED,HIGH);
          delay(MODE_PARTY_DELAY_MS);
          digitalWrite(BTN_LED,LOW);
          btnLedInUse = false;
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
  delay(LED_TIMESTEP_MS);
}
