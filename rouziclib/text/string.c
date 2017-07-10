char *make_string_copy(const char *orig)
{
	char *copy;

	copy = calloc(strlen(orig)+1, sizeof(char));
	strcpy(copy, orig);

	return copy;
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
