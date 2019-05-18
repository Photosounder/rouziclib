char *skip_string(const char *string, const char *skipstring)		// skipstring must be terminated by a %n
{
	int n=0;
	sscanf(string, skipstring, &n);
	return &string[n];
}

char *skip_whitespace(const char *string)
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

char *string_parse_fractional_12(const char *string, double *v)
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

double doztof(const char *string)
{
	double v;

	string_parse_fractional_12(string, &v);

	return v;
}

xy_t doztof_xy(const char *str_x, const char *str_y)
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

int get_string_linecount(char *text, int len)
{
	int i, linecount=0;

	if (text==NULL)
		return linecount;

	if (text[0]=='\0')
		return linecount;

	if (len <= 0)
		len = strlen(text);

	linecount = 1;
	for (i=0; i < len-1; i++)
		if (text[i] == '\n')
			linecount++;

	return linecount;
}

int string_find_start_nth_line(char *text, int len, int n)	// n is the index of the line to find, the char index is returned
{
	int i, il;

	if (text==NULL)
		return -1;

	if (text[0]=='\0')
		return -1;

	if (n <= 0)
		return 0;

	if (len <= 0)
		len = strlen(text);

	il = 0;
	for (i=0; i < len-1; i++)
	{
		if (n == il)
			return i;

		if (text[i] == '\n')
			il++;
	}

	return i;
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

	*linecount = get_string_linecount(text, len);

	array = calloc(*linecount, sizeof(char *));
	array[0] = text;

	for (ia=1, i=0; i < len-1; i++)
		if (text[i] == '\n')
		{
			array[ia] = &text[i+1];
			text[i] = '\0';
			ia++;
		}

	if (text[len-1] == '\n')
		text[len-1] = '\0';

	return array;
}

char *strstr_i(char *fullstr, char *substr)		// case insensitive substring search
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

char *bstrchr(const char *s, int c, int l)	// find first occurrence of c in char s[] for length l, same as memchr?
{
	const char ch = c;

	if (l <= 0)
		return NULL;

	for (; *s != ch; ++s, --l)
		if (l == 0)
			return NULL;

	return (char*) s;
}

char *bstrstr(const char *s1, int l1, const char *s2, int l2)	// find first occurrence of s2[] in s1[] for length l1, same as memmem?
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

#ifdef _WIN32
void *memmem(const uint8_t *l, size_t l_len, const uint8_t *s, size_t s_len)	// like strstr but binary
{
	int i;

	if (l==NULL || s==NULL || l_len<=0 || s_len<=0 || l_len < s_len)
		return NULL;

	if (s_len == 1)		// special case where s_len is 1
		return memchr(l, s[0], l_len);

	for (i=0; i <= l_len - s_len; i++)
		if (l[i] == s[0])
			if (memcmp(&l[i], s, s_len) == 0)
				return &l[i];

	return NULL;
}
#endif

int compare_varlen_word_to_fixlen_word(const char *var, size_t varlen, const char *fix)		// returns 1 if the words are equal
{
	size_t fixlen = strlen(fix);

	if (fixlen != varlen)
		return 0;

	return strncmp(var, fix, varlen)==0;
}

char *find_pattern_in_string(const char *str, const char *pat)	// looks for matches from the end
{
	int i, ip, str_len, pat_len, match;

	if (str==NULL || pat==NULL)
		return NULL;

	str_len = strlen(str);
	pat_len = strlen(pat);

	for (i = str_len-pat_len; i >= 0; i--)
	{
		match = 1;
		for (ip=0; ip < pat_len && match; ip++)
		{
			switch (pat[ip])
			{
				case '\377':			// 0xFF, matches any char
					break;

				case '\376':			// 0xFE, matches any digit
					if (str[i+ip] < '0' || str[i+ip] > '9')
						match = 0;
					break;

				// '\365' and up are available

				default:			// 0x00 to 0xF4, match a valid UTF-8 byte
					if (str[i+ip] != pat[ip])
						match = 0;
					break;
			}
		}

		if (match)
			return &str[i];
	}
	
	return NULL;
}

char *find_date_time_in_string(const char *str)
{
	// pattern matches YYYY-MM-DD<?>hh.mm.ss
	return find_pattern_in_string(str, "\376\376\376\376-\376\376-\376\376\377\376\376.\376\376.\376\376");
}

double parse_timestamp(const char *ts)
{
	double t = NAN, hh=0., mm=0., ss=0.;
	char *p;

	if (p = find_pattern_in_string(ts, "\376\376:\376\376:\376\376"))	// see if it contains hours (HH)
		sscanf(p, "%lg:%lg:%lg", &hh, &mm, &ss);
	else if (p = find_pattern_in_string(ts, "\376:\376\376:\376\376"))	// see if it contains hours (H)
		sscanf(p, "%lg:%lg:%lg", &hh, &mm, &ss);
	else if (p = find_pattern_in_string(ts, "\376\376:\376\376"))		// see if it contains minutes (MM)
		sscanf(p, "%lg:%lg", &mm, &ss);
	else if (p = find_pattern_in_string(ts, "\376:\376\376"))		// see if it contains minutes (M)
		sscanf(p, "%lg:%lg", &mm, &ss);
	else									// it contains only seconds
		sscanf(ts, "%lg", &ss);

	return (hh*60. + mm)*60. + ss;
}