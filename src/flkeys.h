#ifndef FLKEYS_H_
#define FLKEYS_H_

// key sequence struct, used when translating VT100 key sequences
typedef struct {
        int			keysym;
        const char*	str;
} keyseq;

extern const keyseq* cursorkeys;
extern const keyseq* cursorappkeys;
extern const keyseq* keypadkeys;
extern const keyseq* keypadappkeys;
extern const keyseq* otherkeys;

const char* find_key(int keysym, const keyseq* table);

#endif
