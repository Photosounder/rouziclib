void win_reorder_screenshot_pixels(raster_t r)
{
	xyi_t ip;
	int i, invy;
	srgb_t p;

	// Swap channels
	for (i=0; i < mul_x_by_y_xyi(r.dim); i++)
	{
		p = r.srgb[i];

		r.srgb[i].r = p.b;
		r.srgb[i].b = p.r;
	}

	// Flip image upside down
	for (ip.y=0; ip.y < r.dim.y>>1; ip.y++)
	{
		invy = r.dim.y - 1 - ip.y;

		for (ip.x=0; ip.x < r.dim.x; ip.x++)
		{
			p = r.srgb[ip.y*r.dim.x + ip.x];
			r.srgb[ip.y*r.dim.x + ip.x] = r.srgb[invy*r.dim.x + ip.x];
			r.srgb[invy*r.dim.x + ip.x] = p;
		}
	}
}

#ifdef _WIN32
#ifdef RL_GDI32
#include <wingdi.h>
#include <WinUser.h>
#endif
#endif

raster_t *take_desktop_screenshot(int *raster_count)
{
	raster_t *r=NULL;

	// Initialise the returned count
	if (raster_count)
		*raster_count = 0;
	else
		return NULL;

#ifdef _WIN32
#ifdef RL_GDI32
	DISPLAY_DEVICEA dd;
	DEVMODEA dm;
	DWORD dev_id;
	HWND hDesktopWnd=NULL;
	HDC hDesktopDC=NULL, hCaptureDC=NULL;
	HBITMAP *hCaptureBitmap=NULL, hBitmap=NULL;
	HGDIOBJ hPrevBitmap;
	BITMAPINFO bmi;
	xyi_t pos;
	int i, display_count=0, capture_count=0;

	// Count active displays
	for (dev_id=0; ; dev_id++)
	{
		memset(&dd, 0, sizeof(dd));
		dd.cb = sizeof(dd);
		if (!EnumDisplayDevicesA(NULL, dev_id, &dd, 0))
			break;

		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		if ((dd.StateFlags & DISPLAY_DEVICE_ACTIVE) && !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
			display_count++;
	}

	// Allocate the capture and raster arrays
	if (display_count <= 0)
		goto cleanup;
	r = calloc(display_count, sizeof(raster_t));
	hCaptureBitmap = calloc(display_count, sizeof(HBITMAP));

	// Open the desktop DC used for every capture
	hDesktopWnd = GetDesktopWindow();
	hDesktopDC = GetDC(hDesktopWnd);
	if (!hDesktopDC)
		goto cleanup;

	// Capture every display before processing any pixels
	for (dev_id=0; ; dev_id++)
	{
		memset(&dd, 0, sizeof(dd));
		dd.cb = sizeof(dd);
		if (!EnumDisplayDevicesA(NULL, dev_id, &dd, 0))
			break;

		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		if (!(dd.StateFlags & DISPLAY_DEVICE_ACTIVE) || (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) || !EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
			continue;
		if (capture_count >= display_count)
			break;

		r[capture_count].dim = xyi((int) dm.dmPelsWidth, (int) dm.dmPelsHeight);
		pos = xyi((int) dm.dmPosition.x, (int) dm.dmPosition.y);
		hCaptureDC = CreateCompatibleDC(hDesktopDC);
		hBitmap = CreateCompatibleBitmap(hDesktopDC, r[capture_count].dim.x, r[capture_count].dim.y);
		if (hCaptureDC && hBitmap)
		{
			hPrevBitmap = SelectObject(hCaptureDC, hBitmap);
			if (hPrevBitmap && BitBlt(hCaptureDC, 0, 0, r[capture_count].dim.x, r[capture_count].dim.y, hDesktopDC, pos.x, pos.y, (DWORD)0x00CC0020 /*SRCCOPY*/ | (DWORD)0x40000000 /*CAPTUREBLT*/))
				hCaptureBitmap[capture_count++] = hBitmap, hBitmap = NULL;
			if (hPrevBitmap)
				SelectObject(hCaptureDC, hPrevBitmap);
		}
		if (hCaptureDC)	DeleteDC(hCaptureDC), hCaptureDC = NULL;
		if (hBitmap)	DeleteObject(hBitmap), hBitmap = NULL;
	}

	// Convert the captured bitmaps to rasters
	for (i=0; i < capture_count; i++)
	{
		r[i] = make_raster(NULL, r[i].dim, XYI0, IMAGE_USE_SRGB);

		memset(&bmi, 0, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = r[i].dim.x;
		bmi.bmiHeader.biHeight = r[i].dim.y;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		GetDIBits(hDesktopDC, hCaptureBitmap[i], 0, r[i].dim.y, r[i].srgb, &bmi, DIB_RGB_COLORS);
		win_reorder_screenshot_pixels(r[i]);
	}

	*raster_count = capture_count;

cleanup:
	// Free temporary capture resources
	for (i=0; i < capture_count; i++)
		if (hCaptureBitmap[i])
			DeleteObject(hCaptureBitmap[i]);
	if (hDesktopDC)	ReleaseDC(hDesktopWnd, hDesktopDC);
	free(hCaptureBitmap);
	if (*raster_count == 0)
		free_null(&r);
#endif
#endif

	return r;
}
