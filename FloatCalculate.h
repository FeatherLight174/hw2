#ifndef HW2_FLOATCALCULATE_H
#define HW2_FLOATCALCULATE_H

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

// You can modify the following enum and struct
typedef enum {
  NORMALIZED_T = 0,
  DENORMALIZED_T,
  INFINITY_T,
  ZERO_T,
  NAN_T,
} FloatType;

typedef struct {
  uint32_t sign;
  int32_t exponent;
  float mantissa;

  FloatType type;
} Float;

extern const size_t SIGN_BIT;
extern const size_t EXPONENT_BITS;
extern const size_t MANTISSA_BITS;

// You should not modify the signature of this function
void float_add(const char *a, const char *b, char *result);

#endif // HW2_FLOATCALCULATE_H
