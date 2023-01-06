#ifdef _WIN32
int init_midi_input_device(int dev_id, midiin_dev_t *dev, buffer_t *err_log, void *callback_func, void *callback_data)
{
	int ret;

	memset(dev, 0, sizeof(*dev));

	// Check dev_id
	if (dev_id < 0 || dev_id >= midiInGetNumDevs())
	{
		bufprintf(err_log, "Wrong dev_id %d in init_midi_input_device(): there are only %d devices available.\n", dev_id, midiInGetNumDevs());
		return 0;
	}

	// Get info (unused) of the MIDI input device
	MIDIINCAPS dev_info;
	ret = midiInGetDevCaps(dev_id, &dev_info, sizeof(MIDIINCAPS));
	if (ret != MMSYSERR_NOERROR)
	{
		bufprintf(err_log, "MIDI input device probing error: midiInGetDevCaps() returned %d\n", ret);
		return 0;
	}

	// Open device
	ret = midiInOpen(&dev->handle, dev_id, (DWORD_PTR) callback_func, (DWORD_PTR) callback_data, CALLBACK_FUNCTION);
	if (ret != MMSYSERR_NOERROR)
	{
		bufprintf(err_log, "MIDI input device opening error: midiInOpen() returned %d\n", ret);
		return 0;
	}

	// Prepare header (buffer to store stuff)
	dev->header.dwBufferLength = 1024;
	dev->header.lpData = calloc(dev->header.dwBufferLength, 1);
	ret = midiInPrepareHeader(dev->handle, &dev->header, sizeof(MIDIHDR));
	if (ret != MMSYSERR_NOERROR)
	{
		bufprintf(err_log, "MIDI input device preparing error: midiInPrepareHeader() returned %d\n", ret);
		return 0;
	}

	// Add buffer (header)
	ret = midiInAddBuffer(dev->handle, &dev->header, sizeof(MIDIHDR));
	if (ret != MMSYSERR_NOERROR)
	{
		bufprintf(err_log, "MIDI input device buffer adding error: midiInAddBuffer() returned %d\n", ret);
		return 0;
	}

	// Start
	ret = midiInStart(dev->handle);
	if (ret != MMSYSERR_NOERROR)
	{
		bufprintf(err_log, "MIDI input device starting error: midiInStart() returned %d\n", ret);
		return 0;
	}

	return 1;
}

void close_midi_input_device(midiin_dev_t *dev)
{
	midiInStop(dev->handle);
	midiInReset(dev->handle);
	midiInClose(dev->handle);
	midiInUnprepareHeader(dev->handle, &dev->header, sizeof(MIDIHDR));
}
#endif
