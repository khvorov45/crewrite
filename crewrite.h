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
    crw_CTokenKind_Pound,
    crw_CTokenKind_DoubleSlash,
    crw_CTokenKind_SlashStar,
    crw_CTokenKind_WhitespaceNoNewline,
    crw_CTokenKind_EscapedWhitespaceWithNewline,
    crw_CTokenKind_WhitespaceWithNewline,
    crw_CTokenKind_Word,
    crw_CTokenKind_OpenRound,
    crw_CTokenKind_CloseRound,
    crw_CTokenKind_OpenCurly,
    crw_CTokenKind_CloseCurly,
    crw_CTokenKind_OpenAngle,
    crw_CTokenKind_CloseAngle,
    crw_CTokenKind_SingleQuote,
    crw_CTokenKind_DoubleQuote,
    crw_CTokenKind_Semicolon,
    crw_CTokenKind_Comma,
    crw_CTokenKind_Slash,
    crw_CTokenKind_Star,
    crw_CTokenKind_StarSlash,
} crw_CTokenKind;

typedef struct crw_CToken {
    crw_CTokenKind kind;
    crw_Str        str;
} crw_CToken;

typedef struct crw_CTokenIter {
    crw_TokenIter tokenIter;
    crw_CToken    curCToken;
} crw_CTokenIter;

typedef enum crw_CBeforePPChunkKind {
    crw_CBeforePPChunkKind_Invalid,
    crw_CBeforePPChunkKind_Comment,
    crw_CBeforePPChunkKind_PoundInclude,
    crw_CBeforePPChunkKind_PoundDefine,
    crw_CBeforePPChunkKind_Whitespace,
    crw_CBeforePPChunkKind_Code,
} crw_CBeforePPChunkKind;

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

typedef struct crw_Chunk_Comment {
    bool doubleSlash;
} crw_Chunk_Comment;

typedef struct crw_Chunk_TypeDef {
    crw_Str type;
    crw_Str names;
} crw_Chunk_TypeDef;

typedef struct crw_CBeforePPChunk {
    crw_CBeforePPChunkKind kind;
    crw_Str                str;
    union {
        crw_Chunk_PoundInclude poundInclude;
        crw_Chunk_PoundDefine  poundDefine;
        crw_Chunk_Comment      comment;
        crw_Chunk_TypeDef      typeDef;
    };
} crw_CBeforePPChunk;

typedef struct crw_CBeforePPChunkIter {
    crw_CTokenIter     cTokenIter;
    crw_CBeforePPChunk curBeforePPChunk;
} crw_CBeforePPChunkIter;

crw_PUBLICAPI bool                   crw_memeq(const void* ptr1, const void* ptr2, intptr_t len);
crw_PUBLICAPI bool                   crw_streq(crw_Str str1, crw_Str str2);
crw_PUBLICAPI intptr_t               crw_strlen(const char* str);
crw_PUBLICAPI crw_Str                crw_strSlice(crw_Str str, intptr_t from, intptr_t onePastTo);
crw_PUBLICAPI bool                   crw_tokenHasNewline(crw_Token token);
crw_PUBLICAPI crw_TokenIter          crw_createTokenIter(crw_Str str);
crw_PUBLICAPI crw_Status             crw_tokenIterNext(crw_TokenIter* iter);
crw_PUBLICAPI crw_CTokenIter         crw_createCTokenIter(crw_Str str);
crw_PUBLICAPI crw_Status             crw_cTokenIterNext(crw_CTokenIter* iter);
crw_PUBLICAPI crw_Status             crw_tokenIterNextNonWhitespace(crw_TokenIter* iter);
crw_PUBLICAPI crw_Status             crw_tokenIterNextNonWhitespaceOrUnescapedNewline(crw_TokenIter* iter);
crw_PUBLICAPI crw_CBeforePPChunkIter crw_createCBeforePPChunkIter(crw_Str str);
crw_PUBLICAPI crw_Status             crw_cBeforePPChunkIterNext(crw_CBeforePPChunkIter* iter);

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

crw_PUBLICAPI bool
crw_strStarts(crw_Str str1, crw_Str str2) {
    bool result = false;
    if (str1.len >= str2.len) {
        result = crw_memeq(str1.ptr, str2.ptr, str2.len);
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
                            iter->curCToken.kind = crw_CTokenKind_Slash;
                            iter->curCToken.str = iter->tokenIter.curToken.str;
                            crw_TokenIter tokenIterCopy = iter->tokenIter;
                            if (crw_tokenIterNext(&tokenIterCopy)) {
                                if (tokenIterCopy.curToken.kind == crw_TokenKind_Special) {
                                    if (crw_streq(tokenIterCopy.curToken.str, crw_STR("/"))) {
                                        iter->tokenIter = tokenIterCopy;
                                        iter->curCToken.kind = crw_CTokenKind_DoubleSlash;
                                        iter->curCToken.str = crw_strSlice(iter->tokenIter.str, iter->tokenIter.offset - 2, iter->tokenIter.offset);
                                    } else if (crw_streq(tokenIterCopy.curToken.str, crw_STR("*"))) {
                                        iter->tokenIter = tokenIterCopy;
                                        iter->curCToken.kind = crw_CTokenKind_SlashStar;
                                        iter->curCToken.str = crw_strSlice(iter->tokenIter.str, iter->tokenIter.offset - 2, iter->tokenIter.offset);
                                    }
                                }
                            }
                        } break;

                        case '*': {
                            iter->curCToken.kind = crw_CTokenKind_Star;
                            iter->curCToken.str = iter->tokenIter.curToken.str;
                            crw_TokenIter tokenIterCopy = iter->tokenIter;
                            if (crw_tokenIterNext(&tokenIterCopy)) {
                                if (tokenIterCopy.curToken.kind == crw_TokenKind_Special) {
                                    if (crw_streq(tokenIterCopy.curToken.str, crw_STR("/"))) {
                                        iter->tokenIter = tokenIterCopy;
                                        iter->curCToken.kind = crw_CTokenKind_StarSlash;
                                        iter->curCToken.str = crw_strSlice(iter->tokenIter.str, iter->tokenIter.offset - 2, iter->tokenIter.offset);
                                    }
                                }
                            }
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
                        case ',': iter->curCToken = (crw_CToken) {crw_CTokenKind_Comma, iter->tokenIter.curToken.str}; break;
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

crw_PUBLICAPI crw_CBeforePPChunkIter
crw_createCBeforePPChunkIter(crw_Str str) {
    crw_CBeforePPChunkIter iter = {.cTokenIter = crw_createCTokenIter(str)};
    return iter;
}

crw_PUBLICAPI crw_Status
crw_cBeforePPChunkIterNext(crw_CBeforePPChunkIter* iter) {
    crw_Status result = crw_Failure;

    if (crw_cTokenIterNext(&iter->cTokenIter)) {
        result = crw_Success;
        iter->curBeforePPChunk.kind = crw_CBeforePPChunkKind_Invalid;
        intptr_t offsetChunkBegin = iter->cTokenIter.tokenIter.offset - iter->cTokenIter.curCToken.str.len;

        switch (iter->cTokenIter.curCToken.kind) {
            case crw_CTokenKind_Invalid: break;

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
                                        iter->curBeforePPChunk.kind = crw_CBeforePPChunkKind_PoundInclude;
                                        iter->curBeforePPChunk.poundInclude.angleBrackets = angleBrackets;
                                        iter->curBeforePPChunk.poundInclude.path = crw_strSlice(iter->cTokenIter.tokenIter.str, offsetPathStart, iter->cTokenIter.tokenIter.offset - iter->cTokenIter.tokenIter.curToken.str.len);
                                    }
                                }
                            }
                        } else if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("define"))) {
                            crw_cTokenIterSkipRange(&iter->cTokenIter, crw_CTokenKind_WhitespaceNoNewline, crw_CTokenKind_EscapedWhitespaceWithNewline);
                            if (crw_cTokenIterNext(&iter->cTokenIter)) {
                                if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_Word) {
                                    iter->curBeforePPChunk.kind = crw_CBeforePPChunkKind_PoundDefine;
                                    iter->curBeforePPChunk.poundDefine.name = iter->cTokenIter.curCToken.str;
                                    crw_cTokenIterSkipRange(&iter->cTokenIter, crw_CTokenKind_WhitespaceNoNewline, crw_CTokenKind_EscapedWhitespaceWithNewline);
                                    if (crw_cTokenIterNext(&iter->cTokenIter)) {
                                        intptr_t offsetBodyBegin = iter->cTokenIter.tokenIter.offset - iter->cTokenIter.tokenIter.curToken.str.len;
                                        bool     paramList = false;
                                        bool     foundClose = false;
                                        crw_Str  params = {};
                                        if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_OpenRound) {
                                            paramList = true;
                                            intptr_t offsetParamsBegin = iter->cTokenIter.tokenIter.offset;
                                            while (crw_cTokenIterNext(&iter->cTokenIter)) {
                                                if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_CloseRound) {
                                                    foundClose = true;
                                                    break;
                                                }
                                            }
                                            if (foundClose) {
                                                intptr_t offsetParamsEnd = iter->cTokenIter.tokenIter.offset - iter->cTokenIter.tokenIter.curToken.str.len;
                                                params = crw_strSlice(iter->cTokenIter.tokenIter.str, offsetParamsBegin, offsetParamsEnd);

                                                crw_cTokenIterSkipRange(&iter->cTokenIter, crw_CTokenKind_WhitespaceNoNewline, crw_CTokenKind_EscapedWhitespaceWithNewline);
                                                offsetBodyBegin = iter->cTokenIter.tokenIter.offset;
                                            }
                                        }

                                        if (!paramList || foundClose) {
                                            intptr_t onePastOffsetBodyEnd = iter->cTokenIter.tokenIter.str.len;
                                            for (;;) {
                                                if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_WhitespaceWithNewline) {
                                                    onePastOffsetBodyEnd = iter->cTokenIter.tokenIter.offset - iter->cTokenIter.tokenIter.curToken.str.len;
                                                    break;
                                                }
                                                if (crw_cTokenIterNext(&iter->cTokenIter) == crw_Failure) {
                                                    break;
                                                }
                                            }

                                            iter->curBeforePPChunk.poundDefine.paramList = paramList;
                                            iter->curBeforePPChunk.poundDefine.params = params;
                                            iter->curBeforePPChunk.poundDefine.body = crw_strSlice(iter->cTokenIter.tokenIter.str, offsetBodyBegin, onePastOffsetBodyEnd);
                                        }
                                    }
                                }
                            }
                        } else if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("undef"))) {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        } else if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("line"))) {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        } else if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("error"))) {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        } else if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("warning"))) {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        } else if (crw_streq(iter->cTokenIter.curCToken.str, crw_STR("region"))) {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        } else if (crw_strStarts(iter->cTokenIter.curCToken.str, crw_STR("if"))) {
                            // TODO(khvorov)
                            crw_assert(!"unimplemented");
                        }
                    }
                }
            } break;  // pound

            case crw_CTokenKind_DoubleSlash: {
                while (crw_cTokenIterNext(&iter->cTokenIter)) {
                    if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_WhitespaceWithNewline) {
                        break;
                    }
                }
                iter->curBeforePPChunk.kind = crw_CBeforePPChunkKind_Comment;
                iter->curBeforePPChunk.comment.doubleSlash = true;
            } break;

            case crw_CTokenKind_SlashStar: {
                // NOTE(khvorov) C does not allow nested multiline comments
                while (crw_cTokenIterNext(&iter->cTokenIter)) {
                    if (iter->cTokenIter.curCToken.kind == crw_CTokenKind_StarSlash) {
                        break;
                    }
                }
                iter->curBeforePPChunk.kind = crw_CBeforePPChunkKind_Comment;
                iter->curBeforePPChunk.comment.doubleSlash = false;
            } break;

            case crw_CTokenKind_Word:
            case crw_CTokenKind_WhitespaceWithNewline:
            case crw_CTokenKind_WhitespaceNoNewline:
            case crw_CTokenKind_EscapedWhitespaceWithNewline:
            case crw_CTokenKind_OpenRound:
            case crw_CTokenKind_CloseRound:
            case crw_CTokenKind_OpenCurly:
            case crw_CTokenKind_CloseCurly:
            case crw_CTokenKind_OpenAngle:
            case crw_CTokenKind_CloseAngle:
            case crw_CTokenKind_SingleQuote:
            case crw_CTokenKind_DoubleQuote:
            case crw_CTokenKind_Slash:
            case crw_CTokenKind_Star:
            case crw_CTokenKind_StarSlash:
            case crw_CTokenKind_Comma:
            case crw_CTokenKind_Semicolon: {
                bool           sawNonWhitespace = false;
                crw_CTokenIter iterCopy = iter->cTokenIter;
                while (crw_cTokenIterNext(&iterCopy)) {
                    crw_CTokenKind kd = iterCopy.curCToken.kind;
                    bool           stop = false;
                    switch (kd) {
                        case crw_CTokenKind_Invalid:
                        case crw_CTokenKind_Pound:
                        case crw_CTokenKind_DoubleSlash:
                        case crw_CTokenKind_SlashStar:
                            stop = true;
                            break;
                        case crw_CTokenKind_WhitespaceNoNewline:
                        case crw_CTokenKind_EscapedWhitespaceWithNewline:
                        case crw_CTokenKind_WhitespaceWithNewline:
                            break;
                        case crw_CTokenKind_Word:
                        case crw_CTokenKind_OpenRound:
                        case crw_CTokenKind_CloseRound:
                        case crw_CTokenKind_OpenCurly:
                        case crw_CTokenKind_CloseCurly:
                        case crw_CTokenKind_OpenAngle:
                        case crw_CTokenKind_CloseAngle:
                        case crw_CTokenKind_SingleQuote:
                        case crw_CTokenKind_DoubleQuote:
                        case crw_CTokenKind_Semicolon:
                        case crw_CTokenKind_Comma:
                        case crw_CTokenKind_Slash:
                        case crw_CTokenKind_Star:
                        case crw_CTokenKind_StarSlash:
                            sawNonWhitespace = true;
                            break;
                    }
                    if (stop) {
                        break;
                    }
                    iter->cTokenIter = iterCopy;
                }
                iter->curBeforePPChunk.kind = sawNonWhitespace ? crw_CBeforePPChunkKind_Code : crw_CBeforePPChunkKind_Whitespace;
            } break;

        }  // switch token kind

        iter->curBeforePPChunk.str = crw_strSlice(iter->cTokenIter.tokenIter.str, offsetChunkBegin, iter->cTokenIter.tokenIter.offset);
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
