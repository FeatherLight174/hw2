#include "FloatCalculate.h"
#include <stdbool.h>

const size_t SIGN_BIT = 1;
const size_t EXPONENT_BITS = 8;
const size_t MANTISSA_BITS = 23;

static int32_t get_norm_bias(void) { return 1 - (1 << (EXPONENT_BITS - 1)); }

static int32_t get_denorm_bias(void) { return 1 + get_norm_bias(); }

static bool test_rightmost_all_zeros(uint32_t number, size_t bits) {
  uint32_t mask = (1ull << bits) - 1;
  return (number & mask) == 0;
}

static bool test_rightmost_all_ones(uint32_t number, size_t bits) {
  uint32_t mask = (1ull << bits) - 1;
  return (number & mask) == mask;
}

// You can also design a function of your own.
static void build_bitstring(Float input, char *output){
  int temp = 0;
  for(int k =0; k<32;k++){
    output[k]='0';
  }
  if(input.type==INFINITY_T){
    output[0]=input.sign;
    for(int i = 1; i < 9; i++){
      output[i]='1';
    }
  }
  else if(input.type==ZERO_T){
    output[0]=input.sign;
  }
  else if(input.type==NAN_T){
    output[0]=input.sign;
    for(int i = 1; i < 9; i++){
      output[i]='1';
    }
    for(int k = 1; k < 24; k++){
      if(input.mantissa<(float)pow(0.5, k)){
        input.mantissa-=(float)pow(0.5, k);
        output[k + 8]='1';
      }
    }
  }
  else if(input.type==NORMALIZED_T){
    int i = 8;
    int temp1 = input.exponent-get_norm_bias();

    if(input.sign==1){
      output[0]='1';
    }
    else if(input.sign==0){
      output[0]='0';
    }

    while (temp1!=0)
    {
      temp=temp1%2;
      temp1=temp1/2;
      if(temp==1){
        output[i]='1';
      }
      else if(temp==0){
        output[i]='0';
      }
      i--;
    }
    
    
    for(int k = 1; k < 24; k++){
      if(input.mantissa-1>(float)pow(0.5, k)){
        input.mantissa-=(float)pow(0.5, k);
        output[k + 8]='1';
      }
    }

    

  }
  else if(input.type==DENORMALIZED_T){
    output[0]=input.sign;
    for(int k = 1; k < 24; k++){
      if(input.mantissa<(float)pow(0.5, k)){
        input.mantissa-=(float)pow(0.5, k);
        output[k + 8]='1';
      }
    }
  }
}

// You can also design a function of your own.
static Float parse_bitstring(const char *input){
  Float output;
  output.exponent=-126;
  output.mantissa=0;
  output.sign=0;
  output.type=NORMALIZED_T;
  int32_t exponent = 0;
  float mantissa = 0;
  for(int i = 1; i < 9; i++){
    exponent+=(pow(2, 8-i)+1)*((input[i])-48);
  }
  for(int i = 9; i < 32; i++){
    mantissa+=pow(0.5, i - 8)*((int32_t)(input[i])-48);
  }
  if(test_rightmost_all_zeros(exponent, EXPONENT_BITS)){
    output.exponent=exponent+get_denorm_bias();
    output.mantissa=mantissa;
    output.type=DENORMALIZED_T;
  }
  else{
    output.exponent=exponent+get_norm_bias();
    output.mantissa=mantissa + 1;
    output.type=NORMALIZED_T;
  }
  output.sign=(uint32_t)(input[0]-48);
  return output;
}

// You can also design a function of your own.
static Float float_add_impl(Float a, Float b){
  Float output;
  output.exponent=-126;
  output.mantissa=0;
  output.sign=0;
  output.type=NORMALIZED_T;
  float mantissa_sum = 1;
  if(a.exponent<b.exponent||((a.exponent==b.exponent)&&(a.mantissa<b.mantissa))){
    Float temp;
    temp=b;
    b=a;
    a=temp;
  }
  output.sign=a.sign;
  output.exponent=a.exponent;
  int32_t exponent_diff=a.exponent-b.exponent;
  if(a.sign==b.sign){
    mantissa_sum=a.mantissa+b.mantissa*(float)pow(0.5, exponent_diff);
    if(mantissa_sum>=2){
      if(test_rightmost_all_ones(output.exponent-get_norm_bias(), EXPONENT_BITS)){
        output.type=INFINITY_T;
        return output;
      }
      output.mantissa=mantissa_sum*0.5;
      output.exponent+=1;
    }
    else{
      output.mantissa=mantissa_sum;
    }
  }
  else{
    mantissa_sum=a.mantissa-b.mantissa*(float)pow(0.5, exponent_diff);
    while(output.exponent>-126){
      if(mantissa_sum<1){
        mantissa_sum*=2;
        output.exponent-=1;
      }
      else if(mantissa_sum>=1){
        break;
      }
    }
    output.mantissa=mantissa_sum;
  }
  if(output.mantissa==0){
    output.type=ZERO_T;
  }
  else if(output.mantissa<1){
    output.type=DENORMALIZED_T;
  }
  else if(output.mantissa>=1){
    if(output.exponent+get_norm_bias()<255){
      output.type=NORMALIZED_T;
    }
    else if(output.exponent+get_norm_bias()==255){
      if(output.mantissa==1){
        output.type=INFINITY_T;
      }
      else if(output.mantissa>1){
        output.type=NAN_T;
      }
    }
  }
  return output;
}

// You should not modify the signature of this function
void float_add(const char *a, const char *b, char *result) {
  // TODO: Implement this function
  // A possible implementation of the function:
  Float fa = parse_bitstring(a);
  Float fb = parse_bitstring(b);
  Float fresult = float_add_impl(fa, fb);
  build_bitstring(fresult, result);
}