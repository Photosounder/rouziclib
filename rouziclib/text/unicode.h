extern int utf8_char_size(uint8_t *c);
extern int codepoint_utf8_size(uint32_t c);
extern uint32_t utf8_to_unicode32(uint8_t *c, int32_t *index);
extern uint8_t *sprint_unicode(uint8_t *str, uint32_t c);
extern int find_prev_utf8_char(uint8_t *str, int pos);
extern int find_next_utf8_char(uint8_t *str, int pos);

extern size_t strlen_utf16(const uint16_t *str);
extern int utf16_char_size(uint16_t *c);
extern int codepoint_utf16_size(uint32_t c);
extern uint32_t utf16_to_unicode32(uint16_t *c, int32_t *index);
extern uint16_t *sprint_utf16(uint16_t *str, uint32_t c);
extern uint16_t *utf8_to_utf16(const uint8_t *utf8, uint16_t *utf16);
extern size_t strlen_utf16_to_utf8(const uint16_t *str);
extern uint8_t *utf16_to_utf8(const uint16_t *utf16, uint8_t *utf8);

#define utf8_to_wchar(utf8, wchar)	utf8_to_utf16(utf8, wchar)
#define wchar_to_utf8(wchar, utf8)	utf16_to_utf8(wchar, utf8)

/*#ifdef _WIN32
#define utf8_to_wchar(utf8, wchar)	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wchar, sizeof(wchar)/sizeof(*wchar))
#define wchar_to_utf8(wchar, utf8)	WideCharToMultiByte(CP_UTF8, 0, wchar, -1, utf8, sizeof(utf8), NULL, NULL)
#endif*/
