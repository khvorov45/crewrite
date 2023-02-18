#include "cbuild.h"

#define crw_PUBLICAPI static
#define crw_assert(cond) prb_assert(cond)
#include "../crewrite.h"

#define function static

typedef intptr_t isize;

function prb_Str
readFile(prb_Arena* arena, prb_Str path) {
    prb_ReadEntireFileResult res = prb_readEntireFile(arena, path);
    prb_assert(res.success);
    prb_Str result = prb_strFromBytes(res.content);
    return result;
}

function void
test_tokenIter(void) {
    crw_Str   input = crw_STR("int main() {\n\treturn 0;\n}\n");
    crw_Token expectedTokens[] = {
        {crw_TokenKind_Word, crw_STR("int")},
        {crw_TokenKind_Whitespace, crw_STR(" ")},
        {crw_TokenKind_Word, crw_STR("main")},
        {crw_TokenKind_Special, crw_STR("(")},
        {crw_TokenKind_Special, crw_STR(")")},
        {crw_TokenKind_Whitespace, crw_STR(" ")},
        {crw_TokenKind_Special, crw_STR("{")},
        {crw_TokenKind_Whitespace, crw_STR("\n\t")},
        {crw_TokenKind_Word, crw_STR("return")},
        {crw_TokenKind_Whitespace, crw_STR(" ")},
        {crw_TokenKind_Word, crw_STR("0")},
        {crw_TokenKind_Special, crw_STR(";")},
        {crw_TokenKind_Whitespace, crw_STR("\n")},
        {crw_TokenKind_Special, crw_STR("}")},
        {crw_TokenKind_Whitespace, crw_STR("\n")},
    };

    crw_TokenIter tokenIter = crw_createTokenIter(input);
    prb_assert(tokenIter.curToken.kind == crw_TokenKind_Invalid);
    for (isize tokenInd = 0; tokenInd < prb_arrayCount(expectedTokens); tokenInd++) {
        crw_Token expectedToken = expectedTokens[tokenInd];
        prb_assert(crw_tokenIterNext(&tokenIter));
        prb_assert(tokenIter.curToken.kind == expectedToken.kind);
        prb_assert(crw_streq(tokenIter.curToken.str, expectedToken.str));
    }
    prb_assert(crw_tokenIterNext(&tokenIter) == crw_Failure);
}

function void
test_cTokenIter(void) {
    crw_Str    input = crw_STR("#define MAX \\ \n 4\nint main() {\n\treturn 0;\n}\n// comment\n/**/");
    crw_CToken expectedCTokens[] = {
        {crw_CTokenKind_Pound, crw_STR("#")},
        {crw_CTokenKind_Word, crw_STR("define")},
        {crw_CTokenKind_WhitespaceNoNewline, crw_STR(" ")},
        {crw_CTokenKind_Word, crw_STR("MAX")},
        {crw_CTokenKind_WhitespaceNoNewline, crw_STR(" ")},
        {crw_CTokenKind_EscapedWhitespaceWithNewline, crw_STR("\\ \n ")},
        {crw_CTokenKind_Word, crw_STR("4")},
        {crw_CTokenKind_WhitespaceWithNewline, crw_STR("\n")},
        {crw_CTokenKind_Word, crw_STR("int")},
        {crw_CTokenKind_WhitespaceNoNewline, crw_STR(" ")},
        {crw_CTokenKind_Word, crw_STR("main")},
        {crw_CTokenKind_OpenRound, crw_STR("(")},
        {crw_CTokenKind_CloseRound, crw_STR(")")},
        {crw_CTokenKind_WhitespaceNoNewline, crw_STR(" ")},
        {crw_CTokenKind_OpenCurly, crw_STR("{")},
        {crw_CTokenKind_WhitespaceWithNewline, crw_STR("\n\t")},
        {crw_CTokenKind_Word, crw_STR("return")},
        {crw_CTokenKind_WhitespaceNoNewline, crw_STR(" ")},
        {crw_CTokenKind_Word, crw_STR("0")},
        {crw_CTokenKind_Semicolon, crw_STR(";")},
        {crw_CTokenKind_WhitespaceWithNewline, crw_STR("\n")},
        {crw_CTokenKind_CloseCurly, crw_STR("}")},
        {crw_CTokenKind_WhitespaceWithNewline, crw_STR("\n")},
        {crw_CTokenKind_DoubleSlash, crw_STR("//")},
        {crw_CTokenKind_WhitespaceNoNewline, crw_STR(" ")},
        {crw_CTokenKind_Word, crw_STR("comment")},
        {crw_CTokenKind_WhitespaceWithNewline, crw_STR("\n")},
        {crw_CTokenKind_SlashStar, crw_STR("/*")},
        {crw_CTokenKind_StarSlash, crw_STR("*/")},
    };

    crw_CTokenIter cTokenIter = crw_createCTokenIter(input);
    prb_assert(cTokenIter.curCToken.kind == crw_TokenKind_Invalid);
    for (isize tokenInd = 0; tokenInd < prb_arrayCount(expectedCTokens); tokenInd++) {
        crw_CToken expectedCToken = expectedCTokens[tokenInd];
        prb_assert(crw_cTokenIterNext(&cTokenIter));
        prb_assert(cTokenIter.curCToken.kind == expectedCToken.kind);
        if (!crw_streq(cTokenIter.curCToken.str, expectedCToken.str)) {
            prb_assert(!"failed");
        }
    }
    prb_assert(crw_cTokenIterNext(&cTokenIter) == crw_Failure);
}

function void
test_cCBeforePPChunkIter(void) {
    // NOTE(khvorov) Whitespace joining
    {
        crw_Str inputs[] = {crw_STR(" \n "), crw_STR(" \n \\\n \t\n"), crw_STR("\\\n ")};
        for (isize ind = 0; ind < prb_arrayCount(inputs); ind++) {
            crw_Str        input = inputs[ind];
            crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);
            prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Invalid);

            prb_assert(crw_cBeforePPChunkIterNext(&iter));
            prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Whitespace);
            prb_assert(crw_streq(iter.curBeforePPChunk.str, input));

            prb_assert(crw_cBeforePPChunkIterNext(&iter) == crw_Failure);
        }
    }

    {
        crw_Str        input = crw_STR("#include <stdint.h>\n# include \"cbuild.h\"");
        crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Invalid);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundInclude);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("#include <stdint.h>")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundInclude.path, crw_STR("stdint.h")));
        prb_assert(iter.curBeforePPChunk.poundInclude.angleBrackets);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Whitespace);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("\n")));

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundInclude);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("# include \"cbuild.h\"")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundInclude.path, crw_STR("cbuild.h")));
        prb_assert(!iter.curBeforePPChunk.poundInclude.angleBrackets);

        prb_assert(crw_cBeforePPChunkIterNext(&iter) == crw_Failure);
    }

    {
        crw_Str        input = crw_STR("#define MAX\n#define MIN(x, y)\n#define TEMP(s)");
        crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundDefine);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("#define MAX\n")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.name, crw_STR("MAX")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.body, crw_STR("")));
        prb_assert(!iter.curBeforePPChunk.poundDefine.paramList);
        prb_assert(iter.curBeforePPChunk.poundDefine.params.len == 0);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundDefine);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("#define MIN(x, y)\n")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.name, crw_STR("MIN")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.body, crw_STR("")));
        prb_assert(iter.curBeforePPChunk.poundDefine.paramList);
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.params, crw_STR("x, y")));
    }

    {
        crw_Str        input = crw_STR("#define MAX 4\n#define MIN \\\n45");
        crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundDefine);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("#define MAX 4\n")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.name, crw_STR("MAX")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.body, crw_STR("4")));
        prb_assert(!iter.curBeforePPChunk.poundDefine.paramList);
        prb_assert(iter.curBeforePPChunk.poundDefine.params.len == 0);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundDefine);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("#define MIN \\\n45")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.name, crw_STR("MIN")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.body, crw_STR("45")));
        prb_assert(!iter.curBeforePPChunk.poundDefine.paramList);
        prb_assert(iter.curBeforePPChunk.poundDefine.params.len == 0);
    }

    {
        crw_Str        input = crw_STR("#define MAX(x, y) x > y ? x : y");
        crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundDefine);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("#define MAX(x, y) x > y ? x : y")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.name, crw_STR("MAX")));
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.body, crw_STR("x > y ? x : y")));
        prb_assert(iter.curBeforePPChunk.poundDefine.paramList);
        prb_assert(crw_streq(iter.curBeforePPChunk.poundDefine.params, crw_STR("x, y")));
    }

    {
        crw_Str        input = crw_STR("// comment\n// another comment");
        crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Comment);
        prb_assert(iter.curBeforePPChunk.comment.doubleSlash);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("// comment\n")));

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Comment);
        prb_assert(iter.curBeforePPChunk.comment.doubleSlash);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("// another comment")));
    }

    {
        crw_Str        input = crw_STR("/* comment\nline2 *//**/");
        crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Comment);
        prb_assert(!iter.curBeforePPChunk.comment.doubleSlash);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("/* comment\nline2 */")));

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Comment);
        prb_assert(!iter.curBeforePPChunk.comment.doubleSlash);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("/**/")));
    }

    {
        crw_Str        input = crw_STR("// comment\nint main() {return 0;}#define x");
        crw_CBeforePPChunkIter iter = crw_createCBeforePPChunkIter(input);

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Comment);
        prb_assert(iter.curBeforePPChunk.comment.doubleSlash);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("// comment\n")));

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_Code);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("int main() {return 0;}")));

        prb_assert(crw_cBeforePPChunkIterNext(&iter));
        prb_assert(iter.curBeforePPChunk.kind == crw_CBeforePPChunkKind_PoundDefine);
        prb_assert(crw_streq(iter.curBeforePPChunk.str, crw_STR("#define x")));
    }
}

int
main() {
    test_tokenIter();
    test_cTokenIter();
    test_cCBeforePPChunkIter();

    prb_Arena  arena_ = prb_createArenaFromVmem(1 * prb_GIGABYTE);
    prb_Arena* arena = &arena_;

    prb_Str testsDir = prb_getParentDir(arena, prb_STR(__FILE__));
    prb_Str beforeStr = readFile(arena, prb_pathJoin(arena, testsDir, prb_STR("before.c")));

    crw_TokenIter beforeTokenIter = crw_createTokenIter((crw_Str) {beforeStr.ptr, beforeStr.len});
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Invalid);

    return 0;
}
