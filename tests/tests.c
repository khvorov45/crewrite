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
test_cCChunkIter(void) {
    {
        crw_Str        input = crw_STR("#include <stdint.h>\n# include \"cbuild.h\"");
        crw_CChunkIter iter = crw_createCChunkIter(input);
        prb_assert(iter.curCChunk.kind == crw_CChunkKind_Invalid);

        prb_assert(crw_cChunkIterNext(&iter));
        prb_assert(iter.curCChunk.kind == crw_CChunkKind_PoundInclude);
        prb_assert(crw_streq(iter.curCChunk.str, crw_STR("#include <stdint.h>")));
        prb_assert(crw_streq(iter.curCChunk.poundInclude.path, crw_STR("stdint.h")));
        prb_assert(iter.curCChunk.poundInclude.angleBrackets);

        prb_assert(crw_cChunkIterNext(&iter));
        prb_assert(iter.curCChunk.kind == crw_CChunkKind_PoundInclude);
        prb_assert(crw_streq(iter.curCChunk.str, crw_STR("# include \"cbuild.h\"")));
        prb_assert(crw_streq(iter.curCChunk.poundInclude.path, crw_STR("cbuild.h")));
        prb_assert(!iter.curCChunk.poundInclude.angleBrackets);

        prb_assert(crw_cChunkIterNext(&iter) == crw_Failure);
    }

    {
        crw_Str        input = crw_STR("#define MAX 4\n#define MIN \\\n45");
        crw_CChunkIter iter = crw_createCChunkIter(input);

        prb_assert(crw_cChunkIterNext(&iter));
        prb_assert(iter.curCChunk.kind == crw_CChunkKind_PoundDefineConst);
        prb_assert(crw_streq(iter.curCChunk.str, crw_STR("#define MAX 4")));
        prb_assert(crw_streq(iter.curCChunk.poundDefineConst.macro, crw_STR("MAX")));
        prb_assert(crw_streq(iter.curCChunk.poundDefineConst.value, crw_STR("4")));

        prb_assert(crw_cChunkIterNext(&iter));
        prb_assert(iter.curCChunk.kind == crw_CChunkKind_PoundDefineConst);
        prb_assert(crw_streq(iter.curCChunk.str, crw_STR("#define MIN \\\n45")));
        prb_assert(crw_streq(iter.curCChunk.poundDefineConst.macro, crw_STR("MIN")));
        prb_assert(crw_streq(iter.curCChunk.poundDefineConst.value, crw_STR("45")));
    }
}

int
main() {
    test_tokenIter();
    test_cCChunkIter();

    prb_Arena  arena_ = prb_createArenaFromVmem(1 * prb_GIGABYTE);
    prb_Arena* arena = &arena_;

    prb_Str testsDir = prb_getParentDir(arena, prb_STR(__FILE__));
    prb_Str beforeStr = readFile(arena, prb_pathJoin(arena, testsDir, prb_STR("before.c")));

    crw_TokenIter beforeTokenIter = crw_createTokenIter((crw_Str) {beforeStr.ptr, beforeStr.len});
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Invalid);

    return 0;
}
