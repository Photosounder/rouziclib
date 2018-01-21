char *make_string_copy(const char *orig)
{
	char *copy;

	copy = calloc(strlen(orig)+1, sizeof(char));
	strcpy(copy, orig);

	return copy;
}

void strcpy_then_free(char *dest, char *src)
{
	strcpy(dest, src);
	free(src);
}

char *replace_char(char *str, char find, char replace)	// ASCII replacement of one char with another
{
	char *current_pos = strchr(str,find);

	while (current_pos)
	{
		*current_pos = replace;
		current_pos = strchr(current_pos+1, find);
	}

	return str;
}

char *string_tolower(char *str)
{
	char *p = str;

	if (str==NULL)
		return NULL;

	while (*p)
	{
		*p = tolower(*p);
		p++;
	}

	return str;
}

char *sprintf_realloc(char **string, int *alloc_count, const int append, const char *format, ...)	// like sprintf but expands the string alloc if needed
{
	va_list args;
	int len0=0, len1, zero=0;
	char *p=NULL;

	if (string==NULL)				// if there's no string then we create one
		string = &p;				// so that ultimately it's p that will be returned
	
	if (alloc_count==NULL)				// if alloc_count isn't provided
		alloc_count = &zero;			// use 0 which will realloc string to an adequate size

	va_start(args, format);
	len1 = vsnprintf(NULL, 0, format, args);	// gets the printed length without actually printing
	va_end(args);

	if (string)
		if (append && *string)
			len0 = strlen(*string);

	alloc_enough(string, len0+len1+1, alloc_count, sizeof(char), (*alloc_count)==0 ? 1. : 1.5);

	va_start(args, format);
	vsnprintf(&(*string)[len0], *alloc_count, format, args);
	va_end(args);

	return *string;
}
