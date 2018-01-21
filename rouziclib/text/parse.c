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
	{
		snprintf(field, end-string, "%s", string);	// only copy what's in the field
		field[end-string] = 0;
	}

	return 1;
}

char *string_parse_fractional_12(char *string, double *v)
{
	int i, n=0, ret=1, count=0, neg=0;
	double divisor=1., digit;
	char *p = string;

	*v = 0.;

	p = skip_whitespace(p);

	if (p[0] == '-')
	{
		neg = 1;
		p++;
	}

	for (i=0; i<20 && ret==1; i++)
	{
		n=0;
		ret = sscanf(p, "%lf%n", &digit, &n);
		p = &p[n];
		if (ret==1)
		{
			count++;
			*v += digit / divisor;
			divisor *= 12.;
		}

		n=0;
		sscanf(p, ";%n", &n);
		p = &p[n];

		if (p[0]==' ' || p[0]=='\t')	// detect whitespace so that the next sscanf avoids reading the numbers after the whitespace
			ret = 0;
	}

	if (count==0)
		return string;

	if (neg)
		*v = -*v;

	return p;
}

double doztof(char *string)
{
	double v;

	string_parse_fractional_12(string, &v);

	return v;
}

xy_t doztof_xy(char *str_x, char *str_y)
{
	return xy(doztof(str_x), doztof(str_y));
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

char **arrayise_text(char *text, int *linecount)	// turns line breaks into null chars, makes an array of pointers to the beginning of each lines
{
	int i, ia, len;
	char **array;

	*linecount = 0;
	if (text==NULL)
		return NULL;

	len = strlen(text);
	if (len==0)
	{
		array = calloc(1, sizeof(char *));
		array[0] = text;

		return array;
	}

	*linecount = 1;
	for (i=0; i<len-1; i++)
		if (text[i]=='\n')
			(*linecount)++;

	array = calloc(*linecount, sizeof(char *));
	array[0] = text;

	for (ia=1, i=0; i<len-1; i++)
		if (text[i]=='\n')
		{
			array[ia] = &text[i+1];
			text[i] = '\0';
			ia++;
		}

	if (text[len-1]=='\n')
		text[len-1] = '\0';

	return array;
}

char *strstr_i (char *fullstr, char *substr)		// case insensitive substring search
{
	char *fullstr_low, *substr_low, *p, *ret = NULL;

	if (fullstr==NULL || substr==NULL)
		return NULL;

	fullstr_low = string_tolower(make_string_copy(fullstr));	// make lowercase copy
	substr_low = string_tolower(make_string_copy(substr));

	p = strstr(fullstr_low, substr_low);
	if (p)
		ret = fullstr + (p - fullstr_low);

	free (fullstr_low);
	free (substr_low);

	return ret;
}

char *bstrchr(const char *s, int c, int l)	// find first occurrence of c in char s[] for length l
{
	const char ch = c;

	if (l <= 0)
		return NULL;

	for (; *s != ch; ++s, --l)
		if (l == 0)
			return NULL;

	return (char*) s;
}

char *bstrstr(const char *s1, int l1, const char *s2, int l2)	// find first occurrence of s2[] in s1[] for length l1
{
	const char *ss1 = s1;
	const char *ss2 = s2;

	if (l1 <= 0)
		return NULL;
	if (l2 <= 0)
		return (char *) s1;

	// match prefix
	for (; (s1 = bstrchr(s1, *s2, ss1-s1+l1)) != NULL && ss1-s1+l1!=0; ++s1)
	{
		// match rest of prefix
		const char *sc1, *sc2;
		for (sc1 = s1, sc2 = s2; ;)
			if (++sc2 >= ss2+l2)
				return (char *) s1;
			else if (*++sc1 != *sc2)
				break;
	}

	return NULL;
}
