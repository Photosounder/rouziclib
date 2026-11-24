// In-app message boxes drawn with the window manager, replacing native OS message boxes
// message_box() queues a message that stays up until dismissed and returns immediately, type 0 shows an error icon and other types a warning icon
// message_box_yesno() must be called every frame while waiting, it returns -1 until answered, then 1 for Yes or 0 for No once, it shows a warning icon
// message_box_dialogs() must be called every frame before window_manager(), it returns 1 when a message box is shown
// style_func, if not NULL, is called once on each new message box's window to style it like the app's other dialogs
// All of these must be called from the GUI thread

extern void message_box(const char *message, const char *title, int type);
extern int message_box_yesno(const char *message, const char *title);
extern int message_box_dialogs(void (*style_func)(flwindow_t *w));
