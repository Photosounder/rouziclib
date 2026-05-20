#define RL_MINIZ
#include "../../rouziclib/rouziclib.h"
#include "../../rouziclib/rouziclib.c"

int main(int argc, char **argv)
{
	size_t len;

	if (argc < 3)
		fprintf_rl(stdout, "Usage is fileball <output file> <input path 1> ...\n");

	fprintf_rl(stdout, "Making a fileball.\n");

	if (check_for_pattern_at_end_of_string(argv[1], ".fileball"))
		len = fileball_make_uncompressed_file(argv[1], &argv[2], argc-2, -1);
	else if (check_for_pattern_at_end_of_string(argv[1], ".fileball.z"))
		len = fileball_make_z_file(argv[1], &argv[2], argc-2, -1);
	else if (check_for_pattern_at_end_of_string(argv[1], ".fileball.z.h"))
		len = fileball_make_header_file(argv[1], &argv[2], argc-2, -1);
	else
		fprintf_rl(stderr, "The output file should end in either .fileball, .fileball.z or .fileball.z.h\n");

	char size_string[12];
	sprint_size(size_string, len);
	fprintf_rl(stdout, "Output has a size of %s\n", size_string);

	return 0;
}
