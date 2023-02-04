#include "../crewrite.h"
#include "cbuild.h"

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

    prb_Str exampleDir = prb_getParentDir(arena, prb_STR(__FILE__));
    prb_Str exampleFile = prb_pathJoin(arena, exampleDir, prb_STR("example.c"));

    {
        prb_Str exampleContent = readFile(arena, exampleFile);
        for (;;) {
            if (cur == argument) {
                parameter = ?;
                if (parameter.type == argument.type) {
                    if (parameter.isptr && !argument.isptr) {
                        newarg = cur;
                        newarg.isptr = true;
                        insertIntoNew(newarg);
                    }
                }
            }
        }
    }

    prb_Str exampleOut = prb_replaceExt(arena, exampleFile, prb_STR("exe"));
    prb_Str     exampleCompileCmd = prb_fmt(arena, "clang -g -Wall -Wextra %.*s -o %.*s -lX11", prb_LIT(exampleFile), prb_LIT(exampleOut));
    prb_Process exampleCompileProc = prb_createProcess(exampleCompileCmd, (prb_ProcessSpec) {});
    prb_assert(prb_launchProcesses(arena, &exampleCompileProc, 1, prb_Background_No));

    prb_Process exampleLaunchProc = prb_createProcess(exampleOut, (prb_ProcessSpec) {});
    prb_assert(prb_launchProcesses(arena, &exampleLaunchProc, 1, prb_Background_No));

    return 0;
}
