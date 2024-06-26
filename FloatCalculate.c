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
    if(input.sign==1){
      output[0]='1';
    }
    else if(input.sign==0){
      output[0]='0';
    }
  }
  else if(input.type==NAN_T){
    if(input.sign==1){
      output[0]='1';
    }
    else if(input.sign==0){
      output[0]='0';
    }
    for(int i = 1; i < 9; i++){
      output[i]='1';
    }
  }
  else if(input.type==NORMALIZED_T){
    
    
    uint32_t temp1 = input.exponent-get_norm_bias();
    if(input.sign==1){
      output[0]='1';
    }
    else if(input.sign==0){
      output[0]='0';
    }
    int i = 8;
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
      if(i==0){
        break;
      }
    }
    int k = 31;
    while (input.mantissa!=0)
    {
      temp=input.mantissa%2;
      input.mantissa=input.mantissa/2;
      if(temp==1){
        output[k]='1';
      }
      else if(temp==0){
        output[k]='0';
      }
      k--;
      if(k==8){
        break;
      }
    }
  }
  else if(input.type==DENORMALIZED_T){
    if(input.sign==1){
      output[0]='1';
    }
    else if(input.sign==0){
      output[0]='0';
    }
    int k = 31;
    while (input.mantissa!=0)
    {
      temp=input.mantissa%2;
      input.mantissa=input.mantissa/2;
      if(temp==1){
        output[k]='1';
      }
      else if(temp==0){
        output[k]='0';
      }
      k--;
    }
  }
  output[31]='0';
}

// You can also design a function of your own.
static Float parse_bitstring(const char *input){
  Float output;
  output.exponent=-126;
  output.mantissa=0;
  output.sign=0;
  output.type=NORMALIZED_T;
  int32_t exponent = 0;
  uint32_t mantissa = 0;
  for(int i = 1; i < 9; i++){
    uint32_t mul=1;
    for(int k = 0; k<8-i;k++){
       mul*=2;
    }
    exponent+=mul*((input[i])-48);
  }
  for(int i = 9; i < 32; i++){
    uint32_t mul=1;
    for(int k = 0;k<31-i;k++){
      mul*=2;
    }
    mantissa+=mul*((input[i])-48);
  }
  output.mantissa=mantissa;
  if(test_rightmost_all_zeros(exponent, EXPONENT_BITS)){
    output.exponent=exponent+get_denorm_bias();
    output.type=DENORMALIZED_T;
  }
  else{
    output.exponent=exponent+get_norm_bias();
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
  uint32_t mantissa_sum;
  uint32_t POW23 = 1;
  for(int i=0;i<23;i++){
    POW23*=2;
  }
  if(a.exponent<b.exponent||((a.exponent==b.exponent)&&(a.mantissa<b.mantissa))){
    Float temp;
    temp=b;
    b=a;
    a=temp;
  }
  output.sign=a.sign;
  output.exponent=a.exponent;
  int32_t exponent_diff=a.exponent-b.exponent;
  
  if((a.type==NORMALIZED_T)&&(b.type==NORMALIZED_T)){
    uint32_t b_mantissa=(b.mantissa+POW23)*8;
    uint32_t a_mantissa=(a.mantissa+POW23)*8;
    uint32_t sticky=0;
    
    for(int i=0; i<exponent_diff;i++){
      b_mantissa=b_mantissa>>1;
      if(b_mantissa%2==1){
        sticky=1;
      }
    }
    if((b_mantissa%2==0)&&(sticky==1)){
      b_mantissa+=1;
    }
    if(a.sign==b.sign){
      mantissa_sum=a_mantissa+b_mantissa;
      if(test_rightmost_all_ones(output.exponent-get_norm_bias(), EXPONENT_BITS)){
        output.type=INFINITY_T;
        return output;
      }
    }
    else{
      mantissa_sum=a_mantissa-b_mantissa;
    }
  }
  else if((a.type==NORMALIZED_T)&&(b.type==DENORMALIZED_T)){
    uint32_t b_mantissa=(b.mantissa)*8;
    uint32_t a_mantissa=(a.mantissa+POW23)*8;
    uint32_t sticky=0;
    for(int i=0; i<exponent_diff;i++){
      b_mantissa=b_mantissa>>1;
      if(b_mantissa%2==1){
        sticky=1;
      }
    }
    if((b_mantissa%2==0)&&(sticky==1)){
      b_mantissa+=1;
    }
    if(a.sign==b.sign){
      mantissa_sum=a_mantissa+b_mantissa;
      if(test_rightmost_all_ones(output.exponent-get_norm_bias(), EXPONENT_BITS)){
        output.type=INFINITY_T;
        return output;
      }
    }
    else{
      mantissa_sum=a_mantissa-b_mantissa;
    }
  }
  else if((a.type==DENORMALIZED_T)&&(b.type==DENORMALIZED_T)){
    uint32_t b_mantissa=(b.mantissa)*8;
    uint32_t a_mantissa=(a.mantissa)*8;
    if(a.sign==b.sign){
      mantissa_sum=a_mantissa+b_mantissa;
    }
    else{
      mantissa_sum=a_mantissa+b_mantissa;
    }
  }
  output.mantissa=mantissa_sum/8;
  while(output.mantissa>=POW23*2){
    output.mantissa/=2;
    if(output.exponent==128){
      break;
    }
    output.exponent+=1;
  }
  while(output.mantissa<POW23){
    output.mantissa*=2;
    if(output.exponent==-126){
      break;
    }
    output.exponent-=1;
  }
  if(output.mantissa==0){
    output.type=ZERO_T;
  }
  else if(output.mantissa<POW23){
    output.type=DENORMALIZED_T;
  }
  else if(output.mantissa>=POW23){
    if(output.exponent+get_norm_bias()<255){
      output.type=NORMALIZED_T;
    }
    else if(output.exponent+get_norm_bias()==255){
      if(output.mantissa==POW23){
        output.type=INFINITY_T;
      }
      else if(output.mantissa>POW23){
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