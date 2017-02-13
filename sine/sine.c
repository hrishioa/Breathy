#include <stdio.h>
#include <math.h>

#define PI 3.14159265f

float buf_len = 250;
int peak = 156;
int offset = 156;
float span = PI / 2;  // 2 * PI = whole period
float w0 = -PI / 2;

int main()
{
  int i;
  for (i = 0; i < buf_len; i++) {
    if (i % 8 == 0)
      printf("\n");
    printf("%5d", offset + (int)(peak * sin(w0 + span * i / buf_len)));
    if (i < buf_len - 1)
      printf(", ");
    else
      printf("\n");
  }
  return 0;
}
