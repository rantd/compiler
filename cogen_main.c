#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cogen.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "no input file!\n");
		exit(1);
	}
	tokenizer_t t = mk_tokenizer(argv[1]);
	program_t prog = parse_program(t);
    	cogen_program(stdout, prog);
	return 0;
}
