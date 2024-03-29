#ifdef __MSP430G2452__
#  include <msp430g2452.h>
#endif
#ifdef __MSP430G2231__
#  include <msp430g2231.h>
#endif
#include <legacymsp430.h>

#define CURVELEN 498

#if __GNUC__ > 4 || \
  (__GNUC__ == 4 && (__GNUC_MINOR__ > 5 || \
    (__GNUC_MINOR__ == 5 && \
     __GNUC_PATCHLEVEL__ > 2)))
#else
#  error We need msp430-gcc >= 4.5.3
#endif


const unsigned char curve[] = {
    1,     1,     1,     1,     1,     1,     1,     1, 
    1,     1,     1,     1,     1,     1,     1,     1, 
    1,     1,     1,     2,     2,     2,     2,     2, 
    2,     2,     3,     3,     3,     3,     3,     3, 
    4,     4,     4,     4,     4,     5,     5,     5, 
    5,     6,     6,     6,     6,     7,     7,     7, 
    8,     8,     8,     8,     9,     9,     9,    10, 
   10,    10,    11,    11,    11,    12,    12,    13, 
   13,    13,    14,    14,    15,    15,    15,    16, 
   16,    17,    17,    18,    18,    18,    19,    19, 
   20,    20,    21,    21,    22,    22,    23,    23, 
   24,    24,    25,    25,    26,    26,    27,    27, 
   28,    29,    29,    30,    30,    31,    31,    32, 
   33,    33,    34,    34,    35,    36,    36,    37, 
   38,    38,    39,    39,    40,    41,    41,    42, 
   43,    43,    44,    45,    46,    46,    47,    48, 
   48,    49,    50,    50,    51,    52,    53,    53, 
   54,    55,    56,    56,    57,    58,    59,    59, 
   60,    61,    62,    62,    63,    64,    65,    66, 
   66,    67,    68,    69,    70,    70,    71,    72, 
   73,    74,    75,    75,    76,    77,    78,    79, 
   80,    80,    81,    82,    83,    84,    85,    86, 
   87,    87,    88,    89,    90,    91,    92,    93, 
   94,    95,    95,    96,    97,    98,    99,   100, 
  101,   102,   103,   104,   105,   106,   106,   107, 
  108,   109,   110,   111,   112,   113,   114,   115, 
  116,   117,   118,   119,   120,   121,   122,   122, 
  123,   124,   125,   126,   127,   128,   129,   130, 
  131,   132,   133,   134,   135,   136,   137,   138, 
  139,   140,   141,   142,   143,   144,   145,   146, 
  147,   148,   149,   150,   151,   152,   153,   154, 
  155,   156, 
  155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145,
  144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 
  133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 
  122, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 
  112, 111, 110, 109, 108, 107, 106, 106, 105, 104, 103, 
  102, 101, 100, 99, 98, 97, 96, 95, 95, 94, 93, 92, 91, 
  90, 89, 88, 87, 87, 86, 85, 84, 83, 82, 81, 80, 80, 79, 
  78, 77, 76, 75, 75, 74, 73, 72, 71, 70, 70, 69, 68, 67, 
  66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 59, 58, 57, 56, 
  56, 55, 54, 53, 53, 52, 51, 50, 50, 49, 48, 48, 47, 46, 
  46, 45, 44, 43, 43, 42, 41, 41, 40, 39, 39, 38, 38, 37, 
  36, 36, 35, 34, 34, 33, 33, 32, 31, 31, 30, 30, 29, 29, 
  28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 
  21, 20, 20, 19, 19, 18, 18, 18, 17, 17, 16, 16, 15, 15, 
  15, 14, 14, 13, 13, 13, 12, 12, 11, 11, 11, 10, 10, 10,
   9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 
   4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

int pos = 0, padding = 100, offset = 100;   // Index to PWM's duty cycle table (= brightness)

int main(void)
{
  // Stop watchdog
  WDTCTL = WDTPW + WDTHOLD;

  // Set clock to 1 MHz
  DCOCTL= 0;
  BCSCTL1= CALBC1_1MHZ;
  DCOCTL= CALDCO_1MHZ;

  // SMCLK = 1 MHz / 8 = 125 KHz (SLAU144E p.5-15)
  BCSCTL2 |= DIVS_3;

  // Make P1.6 (green led) an output. SLAU144E p.8-3
  P1DIR |= BIT6 | BIT4;

  // P1OUT &= ^(BIT0);
  P1SEL2 |= BIT4;

  // P1.6 = TA0.1 (timer A's output). SLAS694C p.41
  P1SEL |= BIT6 | BIT4;              

  // PWM period = 125 KHz / 625 = 200 Hz
  TACCR0 = 625;

  // Source Timer A from SMCLK (TASSEL_2), up mode (MC_1).
  // Up mode counts up to TACCR0. SLAU144E p.12-20
  TACTL = TASSEL_2 | MC_1;

  // OUTMOD_7 = Reset/set output when the timer counts to TACCR1/TACCR0
  // CCIE = Interrupt when timer counts to TACCR1
  TACCTL1 = OUTMOD_7 | CCIE;
  TACCTL2 = OUTMOD_7;

  // Initial CCR1 (= brightness)
  TACCR1 = 0;
  TACCR2 = 0;

  // LPM0 (shut down the CPU) with interrupts enabled
  __bis_SR_register(CPUOFF | GIE);

  // Silly return to make gcc happy
  return 0;
}

int wrap(int inp) {
  if(inp < 0 || inp > 249) {
      while(inp < 0)
        inp += 250;
    while(inp > 249)
      inp -= 250;
    inp = 249 - inp;
  }
  return inp;
}

int get_curve(int *curpos, int padding) {
  // *(curpos) = *curpos + 1;
  if((*curpos) > CURVELEN && (*curpos) < CURVELEN+padding)
    return 0;
  if((*curpos) > CURVELEN+padding)
    *curpos = 0;

  return curve[*curpos];
}

// This will be called when timer counts to TACCR1.
#ifdef __MSP430G2452__
interrupt(TIMER0_A1_VECTOR) ta1_isr(void)
#endif
#ifdef __MSP430G2231__
interrupt(TIMERA1_VECTOR) ta1_isr(void)
#endif
{
  int offset_step = 1, padding_step = 1; 
  int new_ccr1 = 1, new_ccr2 = 1, pos2 = 0;

  // Clear interrupt flag
  TACCTL1 &= ~CCIFG;

  if(pos == 0) {
    if(offset > CURVELEN || offset < 0) {

      offset_step = -offset_step;

      if(padding > (CURVELEN-1) || padding < 0)
        padding_step = -padding_step;

      padding += padding_step;

    }
    offset = (offset+offset_step);
  }


  pos = (pos+1) % (CURVELEN+padding);

  if(pos >= CURVELEN)
    new_ccr1 = 0;
  else
    new_ccr1 = curve[pos];

  pos2 = (pos+offset) % (CURVELEN+padding);

  if(pos2 >= CURVELEN)
    new_ccr2 = 0;
  else
    new_ccr2 = curve[pos2];



  // if (pos < 500) {
  //   curpos = pos++ >> 1;
  //   new_ccr1 = curve[curpos];
  //   new_ccr2 = curve[wrap(curpos+offset)];
  // } else if (pos < 1000) {
  //   curpos = (999 - pos++) >> 1;
  //   new_ccr1 = curve[curpos];
  //   new_ccr2 = curve[wrap(curpos+offset)];

  // } else {
  //   pos = 0;
  // }
  // Wait to set the new TACCR1 until TAR has gone past it, so that we
  // don't get interrupted again in this period.

  while (TAR <= new_ccr1 || TAR <= new_ccr2) ;
  TACCR1 = new_ccr1;
  TACCR2 = new_ccr2;
}



