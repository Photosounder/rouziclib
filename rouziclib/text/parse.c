char *skip_string(char *string, const char *skipstring)		// skipstring must be terminated by a %n
{
	int n=0;
	sscanf(string, skipstring, &n);
	return &string[n];
}

char *skip_whitespace(char *string)
{
	return skip_string(string, " %n");
}

int string_get_field(char *string, char *delim, int n, char *field)	// copies the Nth field (0 indexed) of string into field
{
	int i;
	char *end;

	for (i=0; i<n; i++)
	{
		string = strstr(string, delim);			// look for the next delimiter

		if (string==NULL)				// if the next delimiter needed isn't found
			return 0;				// 0 means failure

		string += strlen(delim);			// set string to right after the delimiter that indicates the start
	}

	end = strstr(string, delim);				// look for the next delimiter that marks the end of the field

	if (end==NULL)						// if it was the last field
		strcpy(field, string);				// copy all that is left
	else							// otherwise
		snprintf(field, 1+end-string, "%s", string);	// only copy what's in the field

	return 1;
}

char *string_parse_fractional_12(char *string, double *v)
{
	int i, n=0, ret=1, count=0, d[3], neg=0;
	char *p = string;

	*v = 0.;

	p = skip_whitespace(p);

	if (p[0] == '-')
	{
		neg = 1;
		p++;
	}

	for (i=0; i<3 && ret==1; i++)
	{
		n=0;
		ret = sscanf(p, "%d%n", &d[i], &n);
		p = &p[n];
		if (ret==1)
			count++;

		n=0;
		sscanf(p, ";%n", &n);
		p = &p[n];

		if (p[0]==' ' || p[0]=='\t')
			ret = 0;
	}

	if (count==0)
		return string;

	for (i=count-1; i>=0; i--)
		*v = *v/12. + (double) d[i];

	if (neg)
		*v = -*v;

	return p;
}

char *remove_after_char(char *string, char c)
{
	char *p;

	p = strrchr(string, c);
	if (p)
		p[1] = '\0';

	return string;
}

int strlen_until_after_char(char *string, char c)	// length of a string until last occurence of c (included)
{
	char *p;

	p = strrchr(string, c);
	if (p)
		return p-string + 1;
	else
		return 0;
}

char *remove_after_char_copy(char *string, char c)	// makes a cut copy of a string
{
	int len;
	char *cut;

	len = strlen_until_after_char(string, c);
	if (len==0)
		return NULL;

	cut = calloc(len+1, sizeof(char));

	strncpy(cut, string, len);

	return cut;
}
