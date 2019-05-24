// gcc fileball.c -o fileball.exe -lwinmm -w -Os -fwhole-program

#include "../rouziclib/rouziclib.h"
#include "../rouziclib/rouziclib.c"

int main(int argc, char **argv)
{
	int i, ret;

	fprintf_rl(stdout, "Making a fileball.\n");
	fileball_make_uncompressed_file("output.fileball", &argv[1], argc-1, 0);
	fileball_make_z_file("output.fileball.z", &argv[1], argc-1, 0);
	fileball_make_header_file("output.fileball.z.h", &argv[1], argc-1, 0);

	return 0;
}
