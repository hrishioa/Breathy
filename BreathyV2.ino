//####################### BUTTON VALUES ##########################

#define BTN P2_5            // Button pin
#define BTN_LED_ENABLE true      // Enable/disable the button led

#define BTN_LED_CHECK

bool btnLedsInUse = false;

unsigned long btnPressLength = 0;
bool btnPressed = false;
bool finalButtonMode = false;

#define RANDOMNESSPIN       P1_4

//####################### BUTTON MODE CONFIG #####################

#define MIN_MICROS 0
#define MAX_MICROS 1
#define MODE_LED_PIN        2

#define MODELEN    3

#define SPEED_MODE 1
#define BREATH_TYPE_MODE 2

long btnLedModes[][3] = {
  { 0,      1000000, P2_2 },
  { 1000000, 2000000, P2_3 },
  { 2000000, 4000000, P2_4 }
};

unsigned int nextLed = 0;

//----------------------- PARTY MODE CONFIG ##################

#define MODE_PARTY_MINIMUM_MICROS 30000
#define MODE_PARTY_ROUNDS       5
#define MODE_PARTY_DELAY_MS     75
#define MODE_PARTY_ROUND_DELAY_MS 50

//----------------------- MEDITATION MODE CONFIG ##################

bool meditationModeOn = false;

#define MEDITATION_MODE_PRESS_MAX_MICROS 3000000

//####################### BREATH MODE CONFIG ######################

#define REGULAR_BREATHING 0
#define PANTING_BREATHING 1
#define SHORT_BREATHING   2
#define BOX_BREATHING     3

#define BREATHMODELEN     4

#define RANGE_MIN         0
#define RANGE_MAX         1

#define RANGE_MAX_VALUE   100
#define RANGE_MIN_DISTANCE 10 //If random range ends up closer than this, we'll push it

int modeRanges[][3][2] = {
  { //SLOWEST,FASTEST}          // REGULAR
    {100,   500},  // COUNT_MAX
    {2,     3},    // BREATH_HOLD_PERCENT
    {40,    60}    // BREATH_EMPTY_PERCENT
  },
  { //SLOWEST,FASTEST}         // PANTING
    {75,   175},   // COUNT_MAX
    {0,     0},    // BREATH_HOLD_PERCENT
    {40,    60}    // BREATH_EMPTY_PERCENT
  },
  { //SLOWEST,FASTEST}         // PANTING
    {100,   200},   // COUNT_MAX
    {0,     0},    // BREATH_HOLD_PERCENT
    {70,    80}    // BREATH_EMPTY_PERCENT
  },
  { //SLOWEST,FASTEST}         // BOX
    {500,   1340},   // COUNT_MAX
    {33,     33},    // BREATH_HOLD_PERCENT
    {33,    33}    // BREATH_EMPTY_PERCENT
  }
};

//####################### CONTINUOUS SPEED VARIATION ##############

#define ROUNDS_PER_CHANGE       1
#define SPEED_CHANGE_STEP       5

//####################### LED VALUES ##############################

#define LEDLEN                  2            // Number of LEDs

// LED Configuration Table
// Default values are invalid until initialized and marked as updating
int ledConfigs[][14] = {
  {0, 0, 0, 0, 0, 0, P1_6, 0, 1, 500, 0, ROUNDS_PER_CHANGE, 1, 1},
  {0, 0, 0, 0, 0, 0, P2_1, 0, 1, 500, 0, ROUNDS_PER_CHANGE, 1, 1}
};

#define COUNT_MAX               0
#define BREATH_HOLD_PERCENT     1
#define BREATH_EMPTY_PERCENT    2
#define BREATH_HOLD_VALUE       3
#define BREATH_EMPTY_VALUE      4
#define BRIGHTNESS_PERCENT      5
#define LED_PIN                 6
// Dynamic Values
#define COUNTER                 7
#define DIRECTION               8
#define SPEED                   9
#define MODE                    10
#define ROUNDS_TILL_CHANGE      11
#define SPEED_CHANGE_DIRECTION  12
#define UPDATING                13

#define TRANSITION_LOW          0
#define TRANSITION_SMOOTH       1
#define TRANSITION_HIGH         2

#define LED_TIMESTEP_MS         10
#define LED_COUNTER_STEP        2

//####################### WELCOME FLASH ############################

#define WELCOME_FLASH_ENABLED   true
#define WELCOME_FLASH_PULSE_MS  250
#define WELCOME_FLASH_PULSES    2

//####################### START CODE ###############################

void setup() {
  //Set up the LEDs
  for (int i = 0; i < LEDLEN; i++) {
    pinMode(ledConfigs[i][LED_PIN], OUTPUT);
    // Initialize Counters
    ledConfigs[i][COUNTER] = 0;
    ledMode(random(MODELEN), i, TRANSITION_HIGH, 0);
  }

  // Initialize and read the random seed
  pinMode(RANDOMNESSPIN, INPUT);
  randomSeed(analogRead(RANDOMNESSPIN));

  // Enable button LED
  if (BTN_LED_ENABLE) {
    for (int i = 0; i < MODELEN; i++) {
      pinMode(btnLedModes[i][MODE_LED_PIN], OUTPUT);
      digitalWrite(btnLedModes[i][MODE_LED_PIN], !LOW);
    }
  }

  // Flash Welcome
  welcomeFlash();

  // Set up the button as the last thing we do
  pinMode(BTN, INPUT_PULLUP);
  delay(15);
  attachInterrupt(digitalPinToInterrupt(BTN), btnPress, CHANGE);
}

int getRangedConfig(unsigned int mode, int configIndex, float rangeValue) {
  return ((float)(modeRanges[mode][configIndex][RANGE_MAX] -
           modeRanges[mode][configIndex][RANGE_MIN]) * (rangeValue/RANGE_MAX_VALUE)) +
         modeRanges[mode][configIndex][RANGE_MAX];
}

void ledMode(unsigned int mode, int led, int transition, int newSpeed) {
  if(mode != -1)
    ledConfigs[led][MODE] = mode;
  else
    mode = ledConfigs[led][MODE];

  if (newSpeed >= 0) {
    if(newSpeed > ledConfigs[led][SPEED] && newSpeed-ledConfigs[led][SPEED] < RANGE_MIN_DISTANCE) {
      newSpeed += RANGE_MIN_DISTANCE;
    } else if(ledConfigs[led][SPEED] > newSpeed && ledConfigs[led][SPEED]-newSpeed < RANGE_MIN_DISTANCE) {
      newSpeed -= RANGE_MIN_DISTANCE;
    }

    if(newSpeed > RANGE_MAX_VALUE)
      newSpeed = RANGE_MAX_VALUE;
    if(newSpeed < 0)
      newSpeed = 0;


    ledConfigs[led][SPEED] = newSpeed;
  }


  switch (mode) {
    case REGULAR_BREATHING:
      configureLeds(led, transition, 100,
                    getRangedConfig(mode, COUNT_MAX, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_HOLD_PERCENT, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_EMPTY_PERCENT, ledConfigs[led][SPEED]));

      break;
    case PANTING_BREATHING:
      configureLeds(led, transition, 100,
                    getRangedConfig(mode, COUNT_MAX, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_HOLD_PERCENT, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_EMPTY_PERCENT, ledConfigs[led][SPEED]));
      break;
    case SHORT_BREATHING:
      configureLeds(led, transition, 20,
                    getRangedConfig(mode, COUNT_MAX, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_HOLD_PERCENT, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_EMPTY_PERCENT, ledConfigs[led][SPEED]));
      break;
    case BOX_BREATHING:
      configureLeds(led, transition, 100,
                    getRangedConfig(mode, COUNT_MAX, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_HOLD_PERCENT, ledConfigs[led][SPEED]),
                    getRangedConfig(mode, BREATH_EMPTY_PERCENT, ledConfigs[led][SPEED]));
      break;
  }
}

void configureLeds(unsigned int led, int transition, int brightnessPercent, int countMax, int bHoldPercent, int bEmptyPercent) {
  // Take lock
  ledConfigs[led][UPDATING] = 1;

  // Save old params
  int oldCountMax = ledConfigs[led][COUNT_MAX];

  // Load new params
  ledConfigs[led][COUNT_MAX] = countMax;
  ledConfigs[led][BRIGHTNESS_PERCENT] = brightnessPercent;
  ledConfigs[led][BREATH_HOLD_PERCENT] = bHoldPercent;
  ledConfigs[led][BREATH_EMPTY_PERCENT] = bEmptyPercent;

  //dynamic values;
  if (transition == TRANSITION_HIGH) {
    ledConfigs[led][COUNTER] = ledConfigs[led][COUNT_MAX];
    ledConfigs[led][DIRECTION] = -1;
  } else if (transition == TRANSITION_SMOOTH && oldCountMax > 0) {
    ledConfigs[led][COUNTER] = ((float)ledConfigs[led][COUNTER] / oldCountMax) * (float)ledConfigs[led][COUNT_MAX];
  } else { // default is TRANSITION_LOW
    ledConfigs[led][COUNTER] = 0;
    ledConfigs[led][DIRECTION] = 1;
  }

  ledConfigs[led][BREATH_HOLD_VALUE] = ((float)ledConfigs[led][COUNT_MAX] * ((100 - ledConfigs[led][BREATH_HOLD_PERCENT]))) / 100;
  ledConfigs[led][BREATH_EMPTY_VALUE] = ((float)ledConfigs[led][COUNT_MAX] * (ledConfigs[led][BREATH_EMPTY_PERCENT])) / 100;

  // Release lock
  ledConfigs[led][UPDATING] = 0;
}

void welcomeFlash() {
  if (WELCOME_FLASH_ENABLED) {
    for (int i = 0; i < LEDLEN; i++) {
      if (ledConfigs[i][UPDATING] == 1)
        return;
      ledConfigs[i][UPDATING] = 1;
    }

    for (int i = 0; i < WELCOME_FLASH_PULSES; i++) {
      for (int j = 0; j < LEDLEN; j++)
        digitalWrite(ledConfigs[j][LED_PIN], HIGH);
      delay(WELCOME_FLASH_PULSE_MS);
      for (int j = 0; j < LEDLEN; j++)
        digitalWrite(ledConfigs[j][LED_PIN], LOW);
      delay(WELCOME_FLASH_PULSE_MS);
    }

    for (int i = 0; i < LEDLEN; i++)
      ledConfigs[i][UPDATING] = 0;
  }
}

void btnLights() {
  if (!btnLedsInUse) {
    if (btnPressed) {
      unsigned long pLength = micros() - btnPressLength;

      for (int i = 0; i < MODELEN + 1; i++) {
        if (i == MODELEN) {
          if(!finalButtonMode) {
            finalButtonMode = true;
            for (int j = 0, ledOnCount = 0; j < MODELEN; j++) {
              bool ledEnabled = (j==MODELEN-1 && ledOnCount < 2) || (j != MODELEN-1 && (random(MODELEN)<=(MODELEN-2)));
              if(ledEnabled)
                ledOnCount++;
              digitalWrite(btnLedModes[j][MODE_LED_PIN], !ledEnabled);
            }
          }

        }
        else if (pLength > btnLedModes[i][MIN_MICROS] && pLength <= btnLedModes[i][MAX_MICROS]) {
          finalButtonMode = false;
          for (int j = 0; j < MODELEN; j++)
            digitalWrite(btnLedModes[j][MODE_LED_PIN], !(i == j));
          break;
        }
      }
    } else {
      finalButtonMode = false;
      for (int j = 0; j < MODELEN; j++)
        digitalWrite(btnLedModes[j][MODE_LED_PIN], !LOW);
    }
  }
}

void btnPress() {
  if (!btnPressed) {
    // Button is being pressed

    btnPressed = true;
    btnPressLength = micros();
  } else {
    // Button is being released

    btnPressed = false;
    btnPressLength = micros() - btnPressLength;

    newMode(btnPressLength);
  }
}

void newMode(long buttonPressLength) {
  if (buttonPressLength > btnLedModes[MODELEN - 1][MAX_MICROS]) {
    if (!meditationModeOn) {
      meditationMode(true, buttonPressLength-btnLedModes[MODELEN-1][MAX_MICROS]);
      return;      
    }

  }

  if (meditationModeOn) {
    meditationMode(false, -1);
    return;
  }

  if (buttonPressLength < MODE_PARTY_MINIMUM_MICROS) {
    partyMode();
    return;
  }

  int mode;

  for (mode = 1; mode < MODELEN; mode++) {
    if (buttonPressLength > btnLedModes[mode][MIN_MICROS] && buttonPressLength <= btnLedModes[mode][MAX_MICROS]) {
      if(mode == SPEED_MODE) {
        unsigned int speed = random(RANGE_MAX_VALUE);
        ledMode(-1, nextLed, TRANSITION_HIGH, speed);
        nextLed = (nextLed + 1) % LEDLEN;
      } else if (mode == BREATH_TYPE_MODE) {
        int newMode = random(BREATHMODELEN-1);
        if(newMode == ledConfigs[nextLed][MODE])
          newMode++;
        ledMode(newMode, nextLed, TRANSITION_HIGH, -1);
        nextLed = (nextLed + 1) % LEDLEN;
      }
      break;
    }
  }
}

void meditationMode(bool status, float pressLength) {
  meditationModeOn = status;

  if(status) {
      if(pressLength > MEDITATION_MODE_PRESS_MAX_MICROS)
          pressLength = MEDITATION_MODE_PRESS_MAX_MICROS;
      if(pressLength < 0)
          pressLength = 0;

      int meditationSpeed = ((float)pressLength/MEDITATION_MODE_PRESS_MAX_MICROS)*RANGE_MAX_VALUE;

      for (int i = 0; i < LEDLEN; i++) {
          ledMode(BOX_BREATHING, i, TRANSITION_HIGH, meditationSpeed);
      }
  } else {
      for (int i = 0; i < LEDLEN; i++)
          ledMode(random(BREATHMODELEN), i, TRANSITION_SMOOTH, -1);
  }
}

void partyMode() {
  for (int i = 0; i < LEDLEN; i++) {
    if (ledConfigs[i][UPDATING] == 1)
      return;
    ledConfigs[i][UPDATING] = 1;
  }

  for (int round = 0; round < MODE_PARTY_ROUNDS; round++) {
    for (int led = 0; led < LEDLEN + MODELEN; led++) {
      if (led >= LEDLEN) {
        btnLedsInUse = true;
        digitalWrite(btnLedModes[led - LEDLEN][MODE_LED_PIN], !HIGH);
        delay(MODE_PARTY_DELAY_MS*2);
        digitalWrite(btnLedModes[led - LEDLEN][MODE_LED_PIN], !LOW);
        btnLedsInUse = false;
      } else {
        digitalWrite(ledConfigs[led][LED_PIN], HIGH);
        delay(MODE_PARTY_DELAY_MS);
        digitalWrite(ledConfigs[led][LED_PIN], LOW);
      }
    }
    delay(MODE_PARTY_ROUND_DELAY_MS);
  }

  for (int i = 0; i < LEDLEN; i++)
    ledConfigs[i][UPDATING] = 0;
}

void setBrightness(unsigned int led) {
  if (ledConfigs[led][UPDATING] == 1)
    return;

  int brightness = 0;

  if (ledConfigs[led][COUNTER] > ledConfigs[led][BREATH_HOLD_VALUE]) {
    brightness = 255;
  } else if (ledConfigs[led][COUNTER] < ledConfigs[led][BREATH_EMPTY_VALUE]) {
    brightness = 0;
  } else {
    float pos = ((float)ledConfigs[led][COUNTER] - ledConfigs[led][BREATH_EMPTY_VALUE]) / ((float)ledConfigs[led][BREATH_HOLD_VALUE] - ledConfigs[led][BREATH_EMPTY_VALUE]);
    pos = (tanh((5 * (pos - 0.5))) / 2) + 0.5;
    if (ledConfigs[led][BRIGHTNESS_PERCENT] < 100)
      pos = pos * ((float)ledConfigs[led][BRIGHTNESS_PERCENT] / 100.0);
    brightness = pos * 255;
  }

  analogWrite(ledConfigs[led][LED_PIN], brightness);
}

void roundChange(unsigned int led) {
  //Check for probability of mode change (once per full round)
  

  if(!meditationModeOn) {
    unsigned int modeChangeProb = random((RANGE_MAX_VALUE*2)/SPEED_CHANGE_STEP);
    
    if(modeChangeProb < 1) {
      int newMode = random(BREATHMODELEN-1);
      if(newMode == ledConfigs[led][MODE])
        newMode++;
      ledMode(newMode, led, TRANSITION_SMOOTH, -1);
    } else {
      // Change speed
      int newSpeed = ledConfigs[led][SPEED]+(ledConfigs[led][SPEED_CHANGE_DIRECTION]*SPEED_CHANGE_STEP);
      if(newSpeed >= RANGE_MAX_VALUE) {
        newSpeed = RANGE_MAX_VALUE;
        ledConfigs[led][SPEED_CHANGE_DIRECTION] = -1;
      } else if(newSpeed <= 0) {
        newSpeed = 0;
        ledConfigs[led][SPEED_CHANGE_DIRECTION] = 1;
      }
  
      ledMode(-1, led, TRANSITION_SMOOTH, newSpeed);
    }    
  }
}

void stepLed(unsigned int led) {
  ledConfigs[led][COUNTER] += ledConfigs[led][DIRECTION] * LED_COUNTER_STEP;

  if (ledConfigs[led][COUNTER] >= ledConfigs[led][COUNT_MAX]) {
    ledConfigs[led][COUNTER] = ledConfigs[led][COUNT_MAX];
    ledConfigs[led][DIRECTION] = -1;
  } else if (ledConfigs[led][COUNTER] <= 0) {
    ledConfigs[led][ROUNDS_TILL_CHANGE]--;
    if(ledConfigs[led][ROUNDS_TILL_CHANGE] <= 0) {
      ledConfigs[led][ROUNDS_TILL_CHANGE] = 0;
      roundChange(led);
    }
    ledConfigs[led][COUNTER] = 0;
    ledConfigs[led][DIRECTION] = 1;
  }

  setBrightness(led);
}

void stepLeds() {
  for (int i = 0; i < LEDLEN; i++)
    stepLed(i);
}

void loop() {
  stepLeds();
  btnLights();
  delay(LED_TIMESTEP_MS);
}
