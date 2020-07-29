#pragma once
#include <stdint.h>

typedef struct {
	unsigned char* sequence;
	uint32_t number_of_blocks;
	unsigned char bits_in_last_block;
	uint32_t current_block;
	unsigned char current_offset;
	uint32_t period_random;
	uint32_t last_bit;
	unsigned char current_odd;
} MyRegisterState;

void setAdditionalRandom(unsigned char (*_random)());
void mySRandFromStates(uint32_t paramsLength, MyRegisterState** states);
void mySRandFromSeed(uint32_t paramsLength, uint32_t* params, uint32_t* seed);
void mySRandFromParams(uint32_t paramsLength, uint32_t* params);
void mySRand(uint32_t paramsLength);
uint32_t myRand();