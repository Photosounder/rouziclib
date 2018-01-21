#ifdef _WIN32
PROCESS_INFORMATION create_process(const char *cmd)
{
	STARTUPINFO si={0};
	PROCESS_INFORMATION procinf={0};
	char full_cmd[32768];
	wchar_t wcmd[32768];

	si.cb = sizeof(si);

	sprintf(full_cmd, "C:\\Windows\\System32\\cmd.exe /C %s", cmd);
	utf8_to_wchar(full_cmd, wcmd);

	// Start the child process.
	if( !CreateProcessW( NULL,		// No module name (use command line)
				wcmd,		// Command line
				NULL,		// Process handle not inheritable
				NULL,		// Thread handle not inheritable
				FALSE,		// Set handle inheritance to FALSE
				0,		// No creation flags
				NULL,		// Use parent's environment block
				NULL,		// Use parent's starting directory 
				&si,		// Pointer to STARTUPINFO structure
				&procinf)	// Pointer to PROCESS_INFORMATION structure
	  ) 
	{
		fprintf_rl(stderr, "CreateProcessW failed (%d) in create_process(%s)\n", GetLastError(), cmd);
		return procinf;
	}

	return procinf;
/*
fprintf_rl(stdout, "Waiting...\n");
	// Wait until child process exits.
fprintf_rl(stdout, "return %d\n", WaitForSingleObject(procinf.hProcess, 0));
	WaitForSingleObject(procinf.hProcess, INFINITE);
fprintf_rl(stdout, "return %d\n", WaitForSingleObject(procinf.hProcess, 0));
fprintf_rl(stdout, "Done.\n");

	// Close process and thread handles. 
	CloseHandle(procinf.hProcess);
	CloseHandle(procinf.hThread);*/
}
#endif
