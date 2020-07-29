#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include "MyGenerator.h"

static MyRegisterState** registers = NULL;
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
	result |= getByte(cur_byte, new_odd);
	new_odd ^= change_odd_by_byte[cur_byte];
	result <<= half_bits;
	cur_byte = last_bytes;
	result |= getByte(cur_byte, new_odd);
	new_odd ^= change_odd_by_byte[cur_byte];
	*current_odd = new_odd;
	return result;
}

static uint32_t getFrom(unsigned char* input, int32_t bits_before_end, uint32_t current_block, unsigned char current_offset) {
	if (bits_before_end >= 0) {
		return *((uint64_t*)(&(input[current_block]))) >> (bits - current_offset);
	}
	return *((uint32_t*)(&(input[current_block]))) << -bits_before_end | *((uint32_t*)(&(input[0]))) >> (bits + bits_before_end);
}

static void setTo(unsigned char* input, int32_t bits_before_end, uint32_t current_block, unsigned char current_offset, uint32_t result) {
	if (bits_before_end >= 0) {
		unsigned char offset = (bits - current_offset);
		uint64_t word = *((uint64_t*)(&(input[current_block])));
		word &= UINT64_MAX ^ (((uint64_t)UINT32_MAX) << offset);
		word |= ((uint64_t)result) << offset;
		*((uint64_t*)(&(input[current_block]))) = word;
		return;
	}
	unsigned char begin_offset = bits + bits_before_end;
	unsigned char end_offset = -bits_before_end;
	uint32_t begin_word = *((uint32_t*)(&(input[0])));
	begin_word &= UINT32_MAX >> end_offset;
	begin_word |= result << begin_offset;
	*((uint32_t*)(&(input[0]))) = begin_word;
	uint32_t end_word = *((uint32_t*)(&(input[current_block])));
	end_word &= UINT32_MAX << begin_offset;
	end_word |= result >> end_offset;
	*((uint32_t*)(&(input[current_block]))) = end_word;
}

static uint32_t generateMy(unsigned char* input, uint32_t number_of_blocks, unsigned char bits_in_last_block, uint32_t current_block,
	unsigned char current_offset, uint32_t period_random, uint32_t last_bit, unsigned char* current_odd) {
	int32_t bits_before_end = ((number_of_blocks - 1) * 8 + bits_in_last_block) - ((current_block + 4) * 8 + current_offset);
	uint32_t result = getFrom(input, bits_before_end, current_block, current_offset);
	if (last_bit >= period_random) {
		uint32_t bit = bits - (last_bit - period_random) - 1;
		unsigned char old_bit = (result >> bit) & 1;
		unsigned char new_bit = (*add_random)();
		*current_odd ^= new_bit ^ old_bit;
		result &= UINT32_MAX ^ (1 << bit);
		result |= new_bit << bit;
	}
	result = getResult(result, current_odd);
	setTo(input, bits_before_end, current_block, current_offset, result);
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
			}
			else {
				bytes_by_byte_odd[byte] = new_byte;
			}
		}
	}
}

void setAdditionalRandom(unsigned char (*random)()) {
	add_random = random;
}

void mySRandFromStates(uint32_t paramsLength, MyRegisterState** states) {
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
	}
	srand(0);
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
		}
		sequence[state->number_of_blocks - 1] &= UINT32_MAX << (bits - state->bits_in_last_block);
		state->sequence = (unsigned char*)sequence;
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
		state->number_of_blocks = ceil(params[i] / 8.0);
		state->bits_in_last_block = params[i] % 8;
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
	sequence[number_of_blocks - 1] &= UINT32_MAX << (bits - bits_in_last_block);
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
		reg->last_bit += bits;
		result ^= generateMy(reg->sequence, reg->number_of_blocks, reg->bits_in_last_block, reg->current_block, reg->current_offset,
			reg->period_random, reg->last_bit, &(reg->current_odd));
		reg->current_block += sizeof(uint32_t);
		if (reg->current_block >= reg->number_of_blocks) {
			reg->current_block -= reg->number_of_blocks;
			reg->current_offset += 8 - reg->bits_in_last_block;
			if (reg->current_offset >= 8) {
				reg->current_offset -= 8;
			}
		}
		if (reg->last_bit >= reg->period_random) {
			reg->last_bit -= reg->period_random;
		}
	}
	return result;
}