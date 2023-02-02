#include "cbuild.h"

#define crw_PUBLICAPI static
#define crw_assert(cond) prb_assert(cond)
#include "../crewrite.h"

#define function static

function prb_Str
readFile(prb_Arena* arena, prb_Str path) {
    prb_ReadEntireFileResult res = prb_readEntireFile(arena, path);
    prb_assert(res.success);
    prb_Str result = prb_strFromBytes(res.content);
    return result;
}

int
main() {
    prb_Arena  arena_ = prb_createArenaFromVmem(1 * prb_GIGABYTE);
    prb_Arena* arena = &arena_;

    prb_Str testsDir = prb_getParentDir(arena, prb_STR(__FILE__));
    prb_Str beforeStr = readFile(arena, prb_pathJoin(arena, testsDir, prb_STR("before.c")));

    crw_TokenIter beforeTokenIter = crw_createTokenIter((crw_Str) {beforeStr.ptr, beforeStr.len});
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Invalid);

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Word);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR("int")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Whitespace);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR(" ")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Word);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR("main")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Special);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR("(")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Special);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR(")")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Whitespace);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR(" ")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Special);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR("{")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Word);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR("return")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Whitespace);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR(" ")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Word);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR("0")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Special);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR(";")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter));
    prb_assert(beforeTokenIter.curToken.kind == crw_TokenKind_Special);
    prb_assert(crw_streq(beforeTokenIter.curToken.str, crw_STR("}")));

    prb_assert(crw_codeScannerNext(&beforeTokenIter) == crw_Failure);

    return 0;
}
