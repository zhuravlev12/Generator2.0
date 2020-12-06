#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include "MyGenerator.h"

static MyRegisterState** registers = NULL;
static uint32_t registers_count = 0;
static uint32_t current_registers_count = 0;

static uint32_t bits_generated = 0;
static uint32_t bits_before_next_stream = 0;

static unsigned char default_rand() {
	return rand() & 1;
}

static unsigned char (*add_random)() = &default_rand;

static uint16_t bytes_by_byte_even[256 * 256] = { 0 };
static uint16_t bytes_by_byte_odd[256 * 256] = { 0 };

static unsigned char change_odd_by_byte[256 * 256] = { 0 };

static uint32_t default_params[4] = { 127, 8191, 524287, 33554393 };
static uint32_t default_params_length = 4;

static unsigned char iterative_mode = 1;

static const unsigned char bits = sizeof(uint32_t) * 8;
static const unsigned char half_bits = sizeof(uint16_t) * 8;

#pragma declare simd
static uint16_t getByte(uint16_t last_byte, uint16_t current_odd) {
	return current_odd ? bytes_by_byte_odd[last_byte] : bytes_by_byte_even[last_byte];
}

#pragma declare simd
static uint32_t getResult(uint32_t last_bytes, unsigned char* current_odd) {
	uint32_t result = 0;
	unsigned char new_odd = *current_odd;
	uint16_t cur_byte = last_bytes >> half_bits;
	result |= getByte(cur_byte, new_odd);
	new_odd ^= change_odd_by_byte[cur_byte];
	result <<= half_bits;
	cur_byte = last_bytes;
	result |= getByte(cur_byte, new_odd);
	new_odd ^= change_odd_by_byte[cur_byte];
	*current_odd = new_odd;
	return result;
}

#pragma declare simd
static uint32_t getFrom(unsigned char* input, int32_t bits_before_end, uint32_t current_block, unsigned char current_offset) {
	if (bits_before_end >= 0) {
		return *((uint64_t*)(input + current_block)) >> (bits - current_offset);
	}
	return *((uint32_t*)(input + current_block)) << -bits_before_end | *((uint32_t*)(input)) >> (bits + bits_before_end);
}

#pragma declare simd
static void setTo(unsigned char* input, int32_t bits_before_end, uint32_t current_block, unsigned char current_offset, uint32_t result) {
	unsigned char* current_address = input + current_block;
	if (bits_before_end >= 0) {
		unsigned char offset = bits - current_offset;
		uint64_t word = *((uint64_t*)(current_address));
		word &= UINT64_MAX ^ (((uint64_t)UINT32_MAX) << offset);
		word |= ((uint64_t)result) << offset;
		*((uint64_t*)(current_address)) = word;
		return;
	}
	unsigned char begin_offset = bits + bits_before_end;
	unsigned char end_offset = -bits_before_end;
	uint32_t begin_word = *((uint32_t*)(input));
	begin_word &= UINT32_MAX >> end_offset;
	begin_word |= result << begin_offset;
	*((uint32_t*)(input)) = begin_word;
	uint32_t end_word = *((uint32_t*)(current_address));
	end_word &= UINT32_MAX << begin_offset;
	end_word |= result >> end_offset;
	*((uint32_t*)(current_address)) = end_word;
}

#pragma declare simd
static uint32_t generateMy(unsigned char* input, uint32_t number_of_blocks, unsigned char bits_in_last_block, uint32_t current_block,
	unsigned char current_offset, uint32_t period_random, uint32_t last_bit, unsigned char* current_odd) {
	int32_t bits_before_end = (((number_of_blocks - 1) * 8) + bits_in_last_block) - (((current_block + 4) * 8) + current_offset);
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

static void add_next_register(uint32_t* input_seed, uint32_t param_length) {
	if (registers[current_registers_count] == NULL) {
		MyRegisterState* state = (MyRegisterState*)malloc(sizeof(MyRegisterState));
		state->number_of_blocks = ceil(param_length / 8.0);
		uint32_t bits_in_last_block = param_length % 8;
		if (bits_in_last_block == 0) {
			bits_in_last_block = bits;
		}
		state->bits_in_last_block = bits_in_last_block;
		state->period_random = param_length;
		state->current_block = 0;
		state->current_offset = 0;
		state->last_bit = 0;
		state->sequence = NULL;
		registers[current_registers_count] = state;
	}
	if (registers[current_registers_count]->sequence == NULL) {
		MyRegisterState* state = registers[current_registers_count];
		uint32_t* seed = (uint32_t*)malloc(state->number_of_blocks * sizeof(uint32_t));
		if (input_seed != NULL) {
			for (uint32_t i = 0; i < state->number_of_blocks; i++) {
				seed[i] = input_seed[i];
			}
		}
		else {
			MyRegisterState* prev_state = registers[current_registers_count - 1];
			for (uint32_t i = 0; i < state->number_of_blocks; i++) {
				prev_state->last_bit += bits;
				seed[i] = generateMy(prev_state->sequence, prev_state->number_of_blocks, prev_state->bits_in_last_block,
					prev_state->current_block, prev_state->current_offset, prev_state->period_random, prev_state->last_bit,
					&(prev_state->current_odd));
				prev_state->current_block += sizeof(uint32_t);
				if (prev_state->current_block >= prev_state->number_of_blocks) {
					prev_state->current_block -= prev_state->number_of_blocks;
					prev_state->current_offset += 8 - prev_state->bits_in_last_block;
					if (prev_state->current_offset >= 8) {
						prev_state->current_offset -= 8;
					}
				}
				if (prev_state->last_bit >= prev_state->period_random) {
					prev_state->last_bit -= prev_state->period_random;
				}
			}
		}
		seed[state->number_of_blocks - 1] &= UINT32_MAX << (bits - state->bits_in_last_block);
		state->sequence = (unsigned char*)seed;
		unsigned char current_odd = 0;
		for (uint32_t i = 0; i < state->number_of_blocks; i++) {
			for (uint32_t j = 0; j < bits; j++) {
				current_odd ^= (seed[i] >> j) & 1;
			}
		}
		state->current_odd = current_odd;
	}
	current_registers_count++;
}

void mysrand(MyGeneratorInitStruct init) {
	srand(0);
	if (registers != NULL) {
		for (uint32_t i = 0; i < registers_count; i++) {
			free(registers[i]->sequence);
			free(registers[i]);
		}
		free(registers);
	}
	else {
		init_tables();
	}
	current_registers_count = 0;
	iterative_mode = init.iterative_mode;
	if (init.additional_random != NULL) {
		add_random = init.additional_random;
	}
	if (init.states == NULL) {
		if (init.params == NULL) {
			if (init.params_length <= 0 || init.params_length > default_params_length) {
				init.params_length = default_params_length;
			}
			init.params = default_params;
		}
		if (init.seed == NULL) {
			uint32_t number_of_blocks = ceil((float)(init.params[0]) / bits);
			uint32_t bits_in_last_block = init.params[0] % bits;
			if (bits_in_last_block == 0) {
				bits_in_last_block = bits;
			}
			uint32_t* seed = (uint32_t*)malloc(number_of_blocks * sizeof(uint32_t));
			for (uint32_t i = 0; i < number_of_blocks; i++) {
				seed[i] = rand();
			}
			seed[number_of_blocks - 1] &= UINT32_MAX << (bits - bits_in_last_block);
			init.seed = seed;
		}
		registers = (MyRegisterState**)calloc(init.params_length, sizeof(MyRegisterState*));
		add_next_register(init.seed, init.params[0]);
		for (uint32_t i = 1; i < init.params_length; i++) {
			add_next_register(NULL, init.params[i]);
		}
	}
	else {
		registers = (MyRegisterState**)calloc(init.params_length, sizeof(MyRegisterState*));
		for (uint32_t i = 0; i < init.params_length; i++) {
			registers[i] = init.states[i];
		}
	}
	registers_count = init.params_length;
	if (iterative_mode) {
		current_registers_count = 1;
		bits_before_next_stream = (registers[0]->number_of_blocks - 1) * bits + registers[0]->bits_in_last_block;
		bits_generated = 0;
	}
	else {
		current_registers_count = registers_count;
	}
	srand(0);
}

uint32_t myrand() {
	uint32_t result = 0;
	if (iterative_mode && current_registers_count < registers_count) {
		while (bits_generated >= bits_before_next_stream && current_registers_count < registers_count) {
			add_next_register(NULL, 0);
			bits_generated -= bits_before_next_stream;
			bits_before_next_stream = 1;
			for (uint32_t i = 0; i < current_registers_count; i++) {
				bits_before_next_stream *= (registers[i]->number_of_blocks - 1) * bits + registers[i]->bits_in_last_block;
			}
		}
		bits_generated += bits;
	}
#pragma omp simd
	for (uint32_t i = 0; i < current_registers_count; i++) {
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