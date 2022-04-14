#include <inttypes.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "logger-huffman.h"

// @link http://mypractic.ru/ds18b20-datchik-temperatury-s-interfejsom-1-wire-opisanie-na-russkom-yazyke.html#5
#define CNT 14


uint16_t values[CNT] = {
	0x01a9,
	0x07d0,
	0x0550,
	0x0191,
	0x00a2,
	0x0008,
	0x0000,

	0xfff8,
	0xff5e,
	0xfe6f,
	0xfc90,

	0x7322,
	0x072f,
	0x08d0
};

uint16_t values2[6] = {
	0xa801,
	0xa801,
	0xa901,
	0xa901,
	0xa901,
	0xcf06
};

double expected[CNT] = {
	26.5625,
	125.0,
	85.0,
	25.0625,
	10.125,
	0.5,
	0.0,

	-0.5,
	-10.1250,
	-25.0625,
	-55.0,

	1842.125,
    114.9375,
    141.0000
};

/**
 * @see https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format/45041802
 */ 
static char *int2bin(int num, int pad)
{
	char *str = malloc(sizeof(char) * (pad + 1));
  	if (str) {
   		str[pad] = '\0';
		while (--pad>=0) {
			str[pad] = num & 1 ? '1' : '0';
			num >>= 1;
		}
		return str;
  	}
   	return "";
}

int main(int argc, char **argv)
{
	for (int i = 0; i < CNT; i++) {
		uint16_t vv = htobe16(values[i]);
		TEMPERATURE_2_BYTES v = *(TEMPERATURE_2_BYTES*) & vv;
		double t = TEMPERATURE_2_BYTES_2_double(v);
		printf("%4x %s %8.4f must %8.4f\n", values[i], int2bin(values[i], 16), t, expected[i]);
		assert(fabs(t - expected[i]) < 0.001);
	}

	for (int i = 0; i < 6; i++) {
		double t = TEMPERATURE_2_BYTES_2_double(*(TEMPERATURE_2_BYTES*) &values2[i]);
        printf("%4x %s %8.4f\n", values2[i], int2bin(values2[i], 16), t);
	}
}
