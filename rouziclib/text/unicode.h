extern int utf8_char_size(uint8_t *c);
extern int codepoint_utf8_size(uint32_t c);
extern uint32_t utf8_to_unicode32(uint8_t *c, int32_t *index);
extern uint8_t *sprint_unicode(uint8_t *str, uint32_t c);
extern int find_prev_utf8_char(uint8_t *str, int pos);
extern int find_next_utf8_char(uint8_t *str, int pos);
