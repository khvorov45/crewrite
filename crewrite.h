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

typedef enum crw_CTokenKind {
    crw_CTokenKind_Invalid,
    crw_CTokenKind_Word,
    crw_CTokenKind_WhitespaceNoNewline,
    crw_CTokenKind_EscapedWhitespaceWithNewline,
    crw_CTokenKind_WhitespaceWithNewline,
    crw_CTokenKind_OpenRound,
    crw_CTokenKind_CloseRound,
    crw_CTokenKind_OpenCurly,
    crw_CTokenKind_CloseCurly,
    crw_CTokenKind_OpenAngle,
    crw_CTokenKind_CloseAngle,
    crw_CTokenKind_SingleQuote,
    crw_CTokenKind_DoubleQuote,
    crw_CTokenKind_Semicolon,
    crw_CTokenKind_Pound,
} crw_CTokenKind;

typedef struct crw_CToken {
    crw_CTokenKind kind;
    crw_Str        str;
} crw_CToken;

typedef struct crw_CTokenIter {
    crw_TokenIter tokenIter;
    crw_CToken    curCToken;
} crw_CTokenIter;

typedef enum crw_CChunkKind {
    crw_CChunkKind_Invalid,
    crw_CChunkKind_Whitespace,
    crw_CChunkKind_PoundInclude,
    crw_CChunkKind_PoundDefine,
} crw_CChunkKind;

typedef struct crw_Chunk_PoundInclude {
    crw_Str path;
    bool    angleBrackets;
} crw_Chunk_PoundInclude;

typedef struct crw_Chunk_PoundDefine {
    crw_Str name;
    bool    paramList;
    crw_Str params;
    crw_Str body;
} crw_Chunk_PoundDefine;

typedef struct crw_CChunk {
    crw_CChunkKind kind;
    crw_Str        str;
    union {
        crw_Chunk_PoundInclude poundInclude;
        crw_Chunk_PoundDefine  poundDefine;
    };
} crw_CChunk;

typedef struct crw_CChunkIter {
    crw_CTokenIter cTokenIter;
    crw_CChunk     curCChunk;
} crw_CChunkIter;

crw_PUBLICAPI bool           crw_memeq(const void* ptr1, const void* ptr2, intptr_t len);
crw_PUBLICAPI bool           crw_streq(crw_Str str1, crw_Str str2);
crw_PUBLICAPI intptr_t       crw_strlen(const char* str);
crw_PUBLICAPI crw_Str        crw_strSlice(crw_Str str, intptr_t from, intptr_t onePastTo);
crw_PUBLICAPI bool           crw_tokenHasNewline(crw_Token token);
crw_PUBLICAPI crw_TokenIter  crw_createTokenIter(crw_Str str);
crw_PUBLICAPI crw_Status     crw_tokenIterNext(crw_TokenIter* iter);
crw_PUBLICAPI crw_CTokenIter crw_createCTokenIter(crw_Str str);
crw_PUBLICAPI crw_Status     crw_cTokenIterNext(crw_CTokenIter* iter);
crw_PUBLICAPI crw_Status     crw_tokenIterNextNonWhitespace(crw_TokenIter* iter);
crw_PUBLICAPI crw_Status     crw_tokenIterNextNonWhitespaceOrUnescapedNewline(crw_TokenIter* iter);
crw_PUBLICAPI crw_CChunkIter crw_createCChunkIter(crw_Str str);
crw_PUBLICAPI crw_Status     crw_cChunkIterNext(crw_CChunkIter* iter);

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

crw_PUBLICAPI bool
crw_strHasAny(crw_Str str, crw_Str chars) {
    bool found = false;
    for (intptr_t strInd = 0; strInd < str.len; strInd++) {
        char strCh = str.ptr[strInd];
        for (intptr_t charInd = 0; charInd < chars.len; charInd++) {
            char charsCh = chars.ptr[charInd];
            if (strCh == charsCh) {
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    return found;
}

crw_PUBLICAPI bool
crw_tokenHasNewline(crw_Token token) {
    bool result = false;
    if (token.kind == crw_TokenKind_Whitespace) {
        for (intptr_t ind = 0; ind < token.str.len; ind++) {
            char ch = token.str.ptr[ind];
            if (ch == '\r' || ch == '\n') {
                result = true;
                break;
            }
        }
    }
    return result;
}

crw_PUBLICAPI crw_TokenIter
crw_createTokenIter(crw_Str str) {
    crw_TokenIter iter = {.str = str, .curToken.kind = crw_TokenKind_Invalid};
    return iter;
}

crw_PUBLICAPI crw_Status
crw_tokenIterNext(crw_TokenIter* iter) {
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

crw_PUBLICAPI crw_CTokenIter
crw_createCTokenIter(crw_Str str) {
    crw_CTokenIter iter = {.tokenIter = crw_createTokenIter(str)};
    return iter;
}

crw_PUBLICAPI crw_Status
crw_cTokenIterNext(crw_CTokenIter* iter) {
    crw_Status result = crw_Failure;

    if (crw_tokenIterNext(&iter->tokenIter)) {
        result = crw_Success;

        switch (iter->tokenIter.curToken.kind) {
            case crw_TokenKind_Invalid: {
                iter->curCToken = (crw_CToken) {crw_CTokenKind_Invalid, iter->tokenIter.curToken.str};
            } break;

            case crw_TokenKind_Whitespace: {
                iter->curCToken.str = iter->tokenIter.curToken.str;
                iter->curCToken.kind = crw_strHasAny(iter->tokenIter.curToken.str, crw_STR("\r\n")) ? crw_CTokenKind_WhitespaceWithNewline : crw_CTokenKind_WhitespaceNoNewline;
            } break;

            case crw_TokenKind_Special: {
                if (iter->tokenIter.curToken.str.len == 1) {
                    switch (iter->tokenIter.curToken.str.ptr[0]) {
                        case '\\': {
                            crw_TokenIter tokenIterCopy = iter->tokenIter;
                            if (crw_tokenIterNext(&tokenIterCopy)) {
                                if (tokenIterCopy.curToken.kind == crw_TokenKind_Whitespace && crw_strHasAny(tokenIterCopy.curToken.str, crw_STR("\r\n"))) {
                                    iter->curCToken.kind = crw_CTokenKind_EscapedWhitespaceWithNewline;
                                    iter->curCToken.str = iter->tokenIter.curToken.str;
                                    iter->curCToken.str.len += tokenIterCopy.curToken.str.len;
                                    iter->tokenIter = tokenIterCopy;
                                } else {
                                    // TODO(khvorov)
                                    crw_assert(!"unimplemented");
                                }
                            } else {
                                iter->curCToken = (crw_CToken) {crw_CTokenKind_Invalid, iter->tokenIter.curToken.str};
                            }
                        } break;

                        case '/': {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        } break;

                        case '(': iter->curCToken = (crw_CToken) {crw_CTokenKind_OpenRound, iter->tokenIter.curToken.str}; break;
                        case ')': iter->curCToken = (crw_CToken) {crw_CTokenKind_CloseRound, iter->tokenIter.curToken.str}; break;
                        case '{': iter->curCToken = (crw_CToken) {crw_CTokenKind_OpenCurly, iter->tokenIter.curToken.str}; break;
                        case '}': iter->curCToken = (crw_CToken) {crw_CTokenKind_CloseCurly, iter->tokenIter.curToken.str}; break;
                        case '<': iter->curCToken = (crw_CToken) {crw_CTokenKind_OpenAngle, iter->tokenIter.curToken.str}; break;
                        case '>': iter->curCToken = (crw_CToken) {crw_CTokenKind_CloseAngle, iter->tokenIter.curToken.str}; break;
                        case '\'': iter->curCToken = (crw_CToken) {crw_CTokenKind_SingleQuote, iter->tokenIter.curToken.str}; break;
                        case '"': iter->curCToken = (crw_CToken) {crw_CTokenKind_DoubleQuote, iter->tokenIter.curToken.str}; break;
                        case ';': iter->curCToken = (crw_CToken) {crw_CTokenKind_Semicolon, iter->tokenIter.curToken.str}; break;
                        case '#': iter->curCToken = (crw_CToken) {crw_CTokenKind_Pound, iter->tokenIter.curToken.str}; break;
                    }
                } else {
                    iter->curCToken = (crw_CToken) {crw_CTokenKind_Invalid, iter->tokenIter.curToken.str};
                }
            } break;

            case crw_TokenKind_Word: {
                iter->curCToken = (crw_CToken) {crw_CTokenKind_Word, iter->tokenIter.curToken.str};
            } break;
        }
    }

    return result;
}

crw_PUBLICAPI void
crw_cTokenIterSkipRange(crw_CTokenIter* iter, crw_CTokenKind from, crw_CTokenKind to) {
    crw_CTokenIter iterCopy = *iter;
    while (crw_cTokenIterNext(&iterCopy)) {
        crw_CTokenKind kind = iterCopy.curCToken.kind;
        if (kind >= from && kind <= to) {
            *iter = iterCopy;
        } else {
            break;
        }
    }
}

crw_PUBLICAPI crw_CChunkIter
crw_createCChunkIter(crw_Str str) {
    crw_CChunkIter iter = {.cTokenIter = crw_createCTokenIter(str)};
    return iter;
}

crw_PUBLICAPI crw_Status
crw_cChunkIterNext(crw_CChunkIter* iter) {
    crw_Status result = crw_Failure;

    if (crw_cTokenIterNext(&iter->cTokenIter)) {
        result = crw_Success;
        iter->curCChunk.kind = crw_CChunkKind_Invalid;
        intptr_t offsetChunkBegin = iter->cTokenIter.tokenIter.offset - iter->cTokenIter.tokenIter.curToken.str.len;

        switch (iter->cTokenIter.curCToken.kind) {
            case crw_TokenKind_Invalid: break;

            case crw_CTokenKind_WhitespaceWithNewline:
            case crw_CTokenKind_WhitespaceNoNewline:
            case crw_CTokenKind_EscapedWhitespaceWithNewline: {
                iter->curCChunk.kind = crw_CChunkKind_Whitespace;
                crw_cTokenIterSkipRange(&iter->cTokenIter, crw_CTokenKind_WhitespaceNoNewline, crw_CTokenKind_WhitespaceWithNewline);
            } break;

            case crw_CTokenKind_Pound: {
                crw_cTokenIterSkipRange(&iter->cTokenIter, crw_CTokenKind_WhitespaceNoNewline, crw_CTokenKind_EscapedWhitespaceWithNewline);
                if (crw_cTokenIterNext(&iter->cTokenIter)) {
                    if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_Word) {
                        if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("include"))) {
                            crw_cTokenIterSkipRange(&iter->cTokenIter, crw_CTokenKind_WhitespaceNoNewline, crw_CTokenKind_EscapedWhitespaceWithNewline);
                            if (crw_cTokenIterNext(&iter->cTokenIter)) {
                                if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_OpenAngle || iter->cTokenIter.curCToken.kind == crw_CTokenKind_DoubleQuote) {
                                    bool     angleBrackets = iter->cTokenIter.curCToken.kind == crw_CTokenKind_OpenAngle;
                                    intptr_t offsetPathStart = iter->cTokenIter.tokenIter.offset;
                                    bool     foundClose = false;
                                    while (crw_cTokenIterNext(&iter->cTokenIter)) {
                                        crw_CTokenKind targetKind = angleBrackets ? crw_CTokenKind_CloseAngle : crw_CTokenKind_DoubleQuote;
                                        if (iter->cTokenIter.curCToken.kind == targetKind) {
                                            foundClose = true;
                                            break;
                                        } else if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_WhitespaceWithNewline) {
                                            break;
                                        }
                                    }
                                    if (foundClose) {
                                        iter->curCChunk.kind = crw_CChunkKind_PoundInclude;
                                        iter->curCChunk.poundInclude.angleBrackets = angleBrackets;
                                        iter->curCChunk.poundInclude.path = crw_strSlice(iter->cTokenIter.tokenIter.str, offsetPathStart, iter->cTokenIter.tokenIter.offset - iter->cTokenIter.tokenIter.curToken.str.len);
                                    }
                                }
                            }
                        } else if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("define"))) {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        } else {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        }
                    }
                }
            } break;

            case crw_CTokenKind_Word:
            case crw_CTokenKind_OpenRound:
            case crw_CTokenKind_CloseRound:
            case crw_CTokenKind_OpenCurly:
            case crw_CTokenKind_CloseCurly:
            case crw_CTokenKind_OpenAngle:
            case crw_CTokenKind_CloseAngle:
            case crw_CTokenKind_SingleQuote:
            case crw_CTokenKind_DoubleQuote:
            case crw_CTokenKind_Semicolon: {
                // TODO(khvorov)
                crw_assert(!"unimplemented");
            } break;

        }  // switch token kind

        iter->curCChunk.str = crw_strSlice(iter->cTokenIter.tokenIter.str, offsetChunkBegin, iter->cTokenIter.tokenIter.offset);
    }  // get a c token

    return result;
}

#endif  // crw_NO_IMPLEMENTATION

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
