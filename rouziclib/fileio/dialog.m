#include <AppKit/AppKit.h>

void mac_file_dialog_win_filter(NSSavePanel *dialog, const char *win_filter)
{
	char field[32];
	int n = 1;

	if (win_filter==NULL)
		return;

	// Parse the windows filter and put each extension into an array
	NSMutableArray *ma = [[NSMutableArray alloc] init];

	while (string_get_field(win_filter, "\1", n, field))
	{
		[ma addObject:[NSString stringWithUTF8String: &field[2]]];
		n += 2;
	}

	// Convert mutable array to array
	NSArray *array = [NSArray arrayWithArray:ma];
	[ma release];

	// Set the array list of file types to the dialog
	if ([array count])
		[dialog setAllowedFileTypes:array];
}

char *save_file_dialog(char *filter)	// the filter is the UTF-8 Windows filter with \1 instead of \0
{
	char *path=NULL;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

	NSSavePanel *dialog = [NSSavePanel savePanel];
	[dialog setExtensionHidden:NO];

	// Add extension filters to the dialog
	mac_file_dialog_win_filter(dialog, filter);

	// Run the dialog and get the path
	if ([dialog runModal] == NSModalResponseOK)
		path = make_string_copy([[[dialog URL] path] UTF8String]);

	[pool release];
	[keyWindow makeKeyAndOrderFront:nil];
	return path;
}
