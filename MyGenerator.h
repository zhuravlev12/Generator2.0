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

typedef struct {
	uint32_t params_length;
	uint32_t* params;
	uint32_t* seed;
	MyRegisterState** states;
	unsigned char (*additional_random)();
	unsigned char iterative_mode;
} MyGeneratorInitStruct;

void mysrand(MyGeneratorInitStruct init);
uint32_t myrand();