// gcc bin_to_c.c -o bin_to_c.exe -lwinmm -w -Os && cp bin_to_c.exe /bin

#include "../rouziclib/rouziclib.h"
#include "../rouziclib/rouziclib.c"

int main(int argc, char *argv[])
{
	int i;

	printf("Binary-to-C. Argc = %d\n", argc);

	for (i=1; i < argc; i++)
	{
		convert_file_to_header_const_string(argv[i]);
		printf("File '%s' converted.\n", argv[i]);
	}

	return 0;
}
