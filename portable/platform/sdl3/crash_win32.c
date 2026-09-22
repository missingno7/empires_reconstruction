/* crash_win32.c -- print a symbolised backtrace on an unhandled exception
 * (bring-up aid; MSVC PDB symbols through DbgHelp). */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>

#pragma comment(lib, "dbghelp.lib")

static LONG WINAPI crash_filter(EXCEPTION_POINTERS *ep)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    CONTEXT ctx = *ep->ContextRecord;
    STACKFRAME64 frame;
    char buffer[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO *sym = (SYMBOL_INFO *)buffer;
    IMAGEHLP_LINE64 line;
    DWORD displacement_line = 0;
    int depth = 0;

    fprintf(stderr, "*** crash: exception 0x%08lx at %p\n",
            ep->ExceptionRecord->ExceptionCode, ep->ExceptionRecord->ExceptionAddress);
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
        ep->ExceptionRecord->NumberParameters >= 2)
        fprintf(stderr, "    access violation: %s address %p\n",
                ep->ExceptionRecord->ExceptionInformation[0] ? "writing" : "reading",
                (void *)ep->ExceptionRecord->ExceptionInformation[1]);

    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    SymInitialize(process, NULL, TRUE);

    memset(&frame, 0, sizeof frame);
    frame.AddrPC.Offset = ctx.Rip;    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx.Rbp; frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx.Rsp; frame.AddrStack.Mode = AddrModeFlat;

    while (depth++ < 40 &&
           StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame, &ctx, NULL,
                       SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
        DWORD64 displacement = 0;
        if (frame.AddrPC.Offset == 0)
            break;
        memset(buffer, 0, sizeof buffer);
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 255;
        line.SizeOfStruct = sizeof line;
        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, sym)) {
            if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &displacement_line, &line))
                fprintf(stderr, "    %s+0x%llx  (%s:%lu)\n", sym->Name,
                        (unsigned long long)displacement, line.FileName, line.LineNumber);
            else
                fprintf(stderr, "    %s+0x%llx\n", sym->Name, (unsigned long long)displacement);
        } else {
            fprintf(stderr, "    %p\n", (void *)frame.AddrPC.Offset);
        }
    }
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}

void crash_handler_install(void)
{
    SetUnhandledExceptionFilter(crash_filter);
}
#else
void crash_handler_install(void) {}
#endif
