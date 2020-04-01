#ifdef _WIN32
char *save_file_dialog(char *filter)		// the filter must use \1 instead of \0 as separator
{
	wchar_t wpath[_MAX_PATH*2]={0}, *wfilter;
	OPENFILENAMEW ofn={0};
	int i, ret=0;
	
	// Details on the fields at http://msdn.microsoft.com/en-us/library/ms646839(VS.85).aspx
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = GetDesktopWindow();		// NULL might work too
	ofn.nMaxFile = _MAX_PATH*2;
	ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	ofn.lpstrFile = wpath;
	//ofn.lpstrDefExt = utf8_to_utf16(def_ext, NULL);
	wfilter = utf8_to_utf16(filter, NULL);

	// Replace \1 with \0
	for (i=0; wfilter[i]; i++)
		if (wfilter[i] == 1)
			wfilter[i] = 0;
	ofn.lpstrFilter = wfilter;

	ret = GetSaveFileNameW(&ofn);

	free(ofn.lpstrFilter);
	free(ofn.lpstrDefExt);

	if (ret)
		return utf16_to_utf8(wpath, NULL);

	return NULL;
}
#endif
