extern int utf8_char_size(uint8_t *c);
extern uint32_t utf8_to_unicode32(uint8_t *c, int32_t *index);
extern uint8_t *sprint_unicode(uint8_t *str, uint32_t c);
