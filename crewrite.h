#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifndef crw_HEADER
#define crw_HEADER

#include <stdint.h>
#include <stdbool.h>

#ifndef crw_PUBLICAPI
#define crw_PUBLICAPI
#endif

#ifndef crw_assert
#define crw_assert(cond)
#endif

#define crw_STR(x) \
    (crw_Str) { x, crw_strlen(x) }

typedef enum crw_Status {
    crw_Failure,
    crw_Success,
} crw_Status;

typedef struct crw_Str {
    const char* ptr;
    intptr_t    len;
} crw_Str;

typedef enum crw_TokenKind {
    crw_TokenKind_Invalid,
    crw_TokenKind_Whitespace,
    crw_TokenKind_Special,
    crw_TokenKind_Word,
} crw_TokenKind;

typedef struct crw_Token {
    crw_TokenKind kind;
    crw_Str       str;
} crw_Token;

typedef struct crw_TokenIter {
    crw_Str   str;
    intptr_t  offset;
    crw_Token curToken;
} crw_TokenIter;

crw_PUBLICAPI bool          crw_memeq(const void* ptr1, const void* ptr2, intptr_t len);
crw_PUBLICAPI bool          crw_streq(crw_Str str1, crw_Str str2);
crw_PUBLICAPI intptr_t      crw_strlen(const char* str);
crw_PUBLICAPI crw_Str       crw_strSlice(crw_Str str, intptr_t from, intptr_t onePastTo);
crw_PUBLICAPI crw_TokenIter crw_createTokenIter(crw_Str str);
crw_PUBLICAPI crw_Status    crw_tokenIterNext(crw_TokenIter* iter);

#endif  // crw_HEADER

#ifndef crw_NO_IMPLEMENTATION

crw_PUBLICAPI bool
crw_memeq(const void* ptr1, const void* ptr2, intptr_t len) {
    bool result = true;
    for (intptr_t ind = 0; ind < len; ind++) {
        uint8_t b1 = ((uint8_t*)ptr1)[ind];
        uint8_t b2 = ((uint8_t*)ptr2)[ind];
        if (b1 != b2) {
            result = false;
            break;
        }
    }
    return result;
}

crw_PUBLICAPI bool
crw_streq(crw_Str str1, crw_Str str2) {
    bool result = false;
    if (str1.len == str2.len) {
        result = crw_memeq(str1.ptr, str2.ptr, str1.len);
    }
    return result;
}

crw_PUBLICAPI intptr_t
crw_strlen(const char* str) {
    intptr_t result = 0;
    while (str[result] != '\0') {
        result += 1;
    }
    return result;
}

crw_PUBLICAPI crw_Str
crw_strSlice(crw_Str str, intptr_t from, intptr_t onePastTo) {
    crw_assert(from <= onePastTo);
    crw_Str result = {str.ptr + from, onePastTo - from};
    return result;
}

crw_PUBLICAPI crw_TokenIter
crw_createTokenIter(crw_Str str) {
    crw_TokenIter iter = {.str = str};
    return iter;
}

crw_PUBLICAPI crw_Status
crw_codeScannerNext(crw_TokenIter* iter) {
    crw_Status result = crw_Failure;
    crw_Str    strLeft = crw_strSlice(iter->str, iter->offset, iter->str.len);

    if (strLeft.len > 0) {
        result = crw_Success;

        crw_Str whitespace = {.ptr = strLeft.ptr};
        while (strLeft.len > 0) {
            char ch = strLeft.ptr[0];
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' || ch == '\f') {
                whitespace.len += 1;
                strLeft = crw_strSlice(strLeft, 1, strLeft.len);
            } else {
                break;
            }
        }

        if (whitespace.len > 0) {
            iter->curToken = (crw_Token) {.kind = crw_TokenKind_Whitespace, .str = whitespace};
            iter->offset += whitespace.len;
        } else {
            crw_Str word = {.ptr = strLeft.ptr};
            while (strLeft.len > 0) {
                char ch = strLeft.ptr[0];
                bool charIsAlphaNum = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
                if (charIsAlphaNum) {
                    word.len += 1;
                    strLeft = crw_strSlice(strLeft, 1, strLeft.len);
                } else {
                    break;
                }
            }
            if (word.len > 0) {
                iter->curToken = (crw_Token) {.kind = crw_TokenKind_Word, .str = word};
                iter->offset += word.len;
            } else {
                word.len = 1;
                iter->curToken = (crw_Token) {.kind = crw_TokenKind_Special, .str = word};
                iter->offset += word.len;
            }
        }
    }

    return result;
}

#endif  // crw_NO_IMPLEMENTATION

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
