#include <inttypes.h>
#include <stdio.h>
#include <assert.h>
#include "logger-huffman.h"

// @link http://mypractic.ru/ds18b20-datchik-temperatury-s-interfejsom-1-wire-opisanie-na-russkom-yazyke.html#5
#define CNT 11
uint16_t values[CNT] = {
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
	0x7322
};

double expected[CNT] = {
	125.0,
	85.0,
	25.0625,
	10.125,
	0.5,
	0.0,
	-0.5,
	-10.125,
	-25.0625,
	-55.0,
	1842.125
};

int main(int argc, char **argv)
{
	for (int i = 0; i < CNT; i++) {
		double t = temperature_2_double(values[i]);
		printf("%8.4f\n", t);
		assert(fabs(t - expected[i]) < 0.001);
	}
}
