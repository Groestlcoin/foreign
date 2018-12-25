//!!!P

#if defined(_M_HYBRID_X86_ARM64)

__declspec(hybrid_guest)
void
__fastcall
_guard_check_icall_nop (
    _In_ uintptr_t Target
    );

#else

extern
__inline
void
__fastcall
_guard_check_icall_nop (
    _In_ uintptr_t Target
    )

/*++

Routine Description:

    This function performs a no-op check when invoked by the compiler to check
    the integrity of a function pointer for Control Flow Guard (/guard).

Arguments:

    Target - Supplies the function pointer to check.

Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER(Target);
    return;
}

#endif

extern
__inline
int
_guard_icall_checks_enforced (
    VOID
    )

/*++

Routine Description:

    This function determines whether Control Flow Guard is enforced for the
    current module.

Arguments:

    None.

Return Value:

    A nonzero value is returned as the function value if Control Flow Guard
    is enforced for the current module.

--*/

{

    return (ReadPointerNoFence(&__guard_check_icall_fptr) != (PVOID)_guard_check_icall_nop);
}
