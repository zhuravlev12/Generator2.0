#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include "MyGenerator.h"

static MyRegisterState ** registers = NULL;
static uint32_t registersCount = 0;

static unsigned char default_rand() {
	return rand() % 2;
}

static unsigned char (*add_random)() = &default_rand;

static uint16_t bytes_by_byte_even[256 * 256] = { 0 };
static uint16_t bytes_by_byte_odd[256 * 256] = { 0 };

static unsigned char change_odd_by_byte[256 * 256] = { 0 };

static uint32_t default_params[7] = { 257, 2579, 25703, 257003, 2570011, 25700033, 257000011 };

static const unsigned char bits = sizeof(uint32_t) * 8;
static const unsigned char half_bits = sizeof(uint16_t) * 8;

static uint16_t getByte(uint16_t last_byte, uint16_t current_odd) {
	return current_odd ? bytes_by_byte_odd[last_byte] : bytes_by_byte_even[last_byte];
}

static uint32_t getResult(uint32_t last_bytes, unsigned char* current_odd) {
	uint32_t result = 0;
	unsigned char new_odd = *current_odd;
	uint16_t cur_byte = last_bytes >> 16;
	uint16_t byte1 = getByte(cur_byte, new_odd);
	new_odd ^= change_odd_by_byte[cur_byte];
	result |= byte1;
	result <<= half_bits;
	cur_byte = last_bytes;
	uint16_t byte2 = getByte(cur_byte, new_odd);
	new_odd ^= change_odd_by_byte[cur_byte];
	result |= byte2;
	*current_odd = new_odd;
	return result;
}

static uint32_t generateMy(uint32_t* input, uint32_t number_of_blocks, unsigned char bits_in_last_block, uint32_t current_block,
	unsigned char current_offset, uint32_t period_random, uint32_t last_bit, unsigned char* current_odd) {
	uint32_t result;
	uint32_t last_byte;
	unsigned char is_last_block = current_block == (number_of_blocks - 1);
	unsigned char is_pre_last_block = current_block == (number_of_blocks - 2);
	unsigned char difference_between_last_block = bits - bits_in_last_block;
	signed char current_offset_after_last_block = bits_in_last_block - current_offset;
	unsigned char current_offset_in_last_block = bits - current_offset_after_last_block;
	unsigned char current_offset_in_next_block = bits - current_offset;
	if (is_last_block) {
		if (current_offset == 0) {
			last_byte = (input[current_block] << difference_between_last_block) + (input[0] >> bits_in_last_block);
		} else {
			last_byte = (input[current_block] << current_offset_in_last_block) + (input[0] >> current_offset_after_last_block);
		}
	} else {
		if (current_offset == 0) {
			last_byte = input[current_block];
		} else {
			last_byte = input[current_block] << current_offset;
			if (is_pre_last_block) {
				last_byte += input[current_block + 1] >> current_offset_after_last_block;
			} else {
				last_byte += input[current_block + 1] >> current_offset_in_next_block;
			}
		}
	}
	result = getResult(last_byte, current_odd);
	last_bit += bits;
	while (last_bit >= period_random) {
		last_bit -= period_random;
		unsigned char old_bit = (result >> last_bit) & 1;
		unsigned char new_bit = (*add_random)();
		*current_odd ^= new_bit ^ old_bit;
		result &= UINT32_MAX - (1 << last_bit);
		result |= new_bit << last_bit;
	}
	if (is_last_block) {
		input[current_block] = ((input[current_block] & (UINT32_MAX << current_offset_after_last_block)) |
			(result >> current_offset_in_last_block)) &
			(UINT32_MAX >> difference_between_last_block);
		input[0] = (input[0] & (UINT32_MAX >> current_offset_in_last_block)) |
			(result << current_offset_after_last_block);
	} else {
		if (current_offset == 0) {
			input[current_block] = result;
		} else {
			input[current_block] = (input[current_block] & (UINT32_MAX << current_offset_in_next_block)) |
				(result >> current_offset);
			if (is_pre_last_block) {
				input[current_block + 1] = (input[current_block + 1] & (UINT32_MAX >> current_offset_in_last_block)) |
					(result << current_offset_after_last_block);
			} else {
				input[current_block + 1] = (input[current_block + 1] & (UINT32_MAX >> current_offset)) |
					(result << current_offset_in_next_block);
			}
		}
	}
	return result;
};

static void init_tables() {
	for (unsigned char odd = 0; odd < 2; odd++) {
		for (uint32_t i = 0; i < 256 * 256; i++) {
			uint16_t byte = i;
			unsigned char current_odd = odd;
			uint16_t new_byte = 0;
			for (unsigned char j = 0; j < half_bits; j++) {
				new_byte <<= 1;
				new_byte ^= current_odd;
				current_odd ^= (byte >> (half_bits - j - 1)) & 1;
			}
			if (odd == 0) {
				bytes_by_byte_even[byte] = new_byte;
				change_odd_by_byte[byte] = current_odd;
			} else {
				bytes_by_byte_odd[byte] = new_byte;
			}
		}
	}
}

void setAdditionalRandom(unsigned char (*random)()) {
	add_random = random;
}

void mySRandFromStates(uint32_t paramsLength, MyRegisterState ** states) {
	if (registers != NULL) {
		for (uint32_t i = 0; i < registersCount; i++) {
			free(registers[i]->sequence);
			free(registers[i]);
		}
		free(registers);
	}
	else {
		init_tables();
	}
	registers = (MyRegisterState**)malloc(paramsLength * sizeof(MyRegisterState*));
	for (uint32_t i = 0; i < paramsLength; i++) {
		registers[i] = states[i];
	}
	registersCount = paramsLength;
	srand(0);
}

void mySRandFromSeed(uint32_t paramsLength, uint32_t* params, uint32_t* seed) {
	if (registers == NULL) {
		init_tables();
		srand(0);
	}
	MyRegisterState** states = (MyRegisterState**)malloc(paramsLength * sizeof(MyRegisterState*));
	for (uint32_t i = 0; i < paramsLength; i++) {
		states[i] = (MyRegisterState*)malloc(sizeof(MyRegisterState));
		MyRegisterState* state = states[i];
		state->number_of_blocks = ceil((float)params[i] / bits);
		uint32_t bits_in_last_block = params[i] % bits;
		if (bits_in_last_block == 0) {
			bits_in_last_block = bits;
		}
		state->bits_in_last_block = bits_in_last_block;
		state->period_random = params[i];
		uint32_t* sequence = (uint32_t*)malloc(state->number_of_blocks * sizeof(uint32_t));
		if (i == 0) {
			for (uint32_t j = 0; j < state->number_of_blocks; j++) {
				sequence[j] = seed[j];
			}
		}
		else {
			MyRegisterState* prev_state = states[i - 1];
			for (uint32_t j = 0; j < state->number_of_blocks; j++) {
				sequence[j] = generateMy(prev_state->sequence, prev_state->number_of_blocks, prev_state->bits_in_last_block,
					prev_state->current_block, prev_state->current_offset, prev_state->period_random, prev_state->last_bit,
					&(prev_state->current_odd));
				prev_state->current_block++;
				prev_state->current_block %= prev_state->number_of_blocks;
				if (prev_state->current_block == 0) {
					prev_state->current_offset += bits - prev_state->bits_in_last_block;
					prev_state->current_offset %= bits;
				}
				prev_state->last_bit += bits;
				prev_state->last_bit %= prev_state->period_random;
			}
			sequence[state->number_of_blocks - 1] >>= (bits - state->bits_in_last_block);
		}
		state->sequence = sequence;
		state->current_block = 0;
		state->current_offset = 0;
		state->last_bit = 0;
		unsigned char current_odd = 0;
		for (uint32_t j = 0; j < state->number_of_blocks; j++) {
			for (uint32_t k = 0; k < bits; k++) {
				current_odd ^= (sequence[j] >> k) & 1;
			}
		}
		state->current_odd = current_odd;
	}
	mySRandFromStates(paramsLength, states);
	free(states);
}

void mySRandFromParams(uint32_t paramsLength, uint32_t* params) {
	uint32_t number_of_blocks = ceil((float)(params[0]) / bits);
	uint32_t bits_in_last_block = params[0] % bits;
	if (bits_in_last_block == 0) {
		bits_in_last_block = bits;
	}
	uint32_t* sequence = (uint32_t*)malloc(number_of_blocks * sizeof(uint32_t));
	for (uint32_t i = 0; i < number_of_blocks; i++) {
		sequence[i] = rand();
	}
	sequence[number_of_blocks - 1] >>= (bits - bits_in_last_block);
	mySRandFromSeed(paramsLength, params, sequence);
}

void mySRand(uint32_t paramsLength) {
	if (paramsLength > 7) {
		paramsLength = 7;
	}
	mySRandFromParams(paramsLength, default_params);
}

uint32_t myRand() {
	uint32_t result = 0;
	for (uint32_t i = 0; i < registersCount; i++) {
		MyRegisterState* reg = registers[i];
		result ^= generateMy(reg->sequence, reg->number_of_blocks, reg->bits_in_last_block, reg->current_block, reg->current_offset,
			reg->period_random, reg->last_bit, &(reg->current_odd));
		if (reg->current_block != reg->number_of_blocks - 1) {
			reg->current_block++;
		} else {
			reg->current_block = 0;
		}
		if (reg->current_block == 0) {
			reg->current_offset += bits - reg->bits_in_last_block;
			if (reg->current_offset >= bits) {
				reg->current_offset -= bits;
			}
		}
		reg->last_bit += bits;
		if (reg->last_bit >= reg->period_random) {
			reg->last_bit -= reg->period_random;
		}
	}
	return result;
}