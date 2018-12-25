#include <fenv.h>
#include "_fenvutils.h"

#include <trans.h>

// produce a machine dependent fp control word
//	Entry:
//		abstr:	abstract control word
uint16_t __get_machine_control(uint32_t abstr) {
	//
	// Set standard infinity and denormal control bits
	//

	unsigned short cw = 0;

	//
	// Set exception mask bits
	//

	if (abstr & _EM_INVALID)
		cw |= IEM_INVALID;
	if (abstr & _EM_ZERODIVIDE)
		cw |= IEM_ZERODIVIDE;
	if (abstr & _EM_OVERFLOW)
		cw |= IEM_OVERFLOW;
	if (abstr & _EM_UNDERFLOW)
		cw |= IEM_UNDERFLOW;
	if (abstr & _EM_INEXACT)
		cw |= IEM_INEXACT;
	if (abstr & _EM_DENORMAL)
		cw |= IEM_DENORMAL;

	//
	// Set rounding mode
	//

	switch (abstr & _MCW_RC) {
	case _RC_NEAR:
		cw |= IRC_NEAR;
		break;
	case _RC_UP:
		cw |= IRC_UP;
		break;
	case _RC_DOWN:
		cw |= IRC_DOWN;
		break;
	case _RC_CHOP:
		cw |= IRC_CHOP;
		break;
	}

	//
	// Set Precision mode
	//

	switch (abstr & _MCW_PC) {
	case _PC_64:
		cw |= IPC_64;
		break;
	case _PC_53:
		cw |= IPC_53;
		break;
	case _PC_24:
		cw |= IPC_24;
		break;
	}


	//
	// Set Infinity mode
	//

	if (abstr & _MCW_IC) {
		cw |= IIC_AFFINE;
	}

	return cw;
}

uint16_t __get_machine_status(uint8_t abstr) {
	uint16_t sw = 0;
	if (abstr & _SW_INVALID)
		sw |= ISW_INVALID;
	if (abstr & _SW_ZERODIVIDE)
		sw |= ISW_ZERODIVIDE;
	if (abstr & _SW_OVERFLOW)
		sw |= ISW_OVERFLOW;
	if (abstr & _SW_UNDERFLOW)
		sw |= ISW_UNDERFLOW;
	if (abstr & _SW_INEXACT)
		sw |= ISW_INEXACT;
	return sw;
}

uint32_t __get_abstract_control_x87(uint16_t cw) {
	unsigned int abstr = 0;


	//
	// Set exception mask bits
	//

	if (cw & IEM_INVALID)
		abstr |= _EM_INVALID;
	if (cw & IEM_ZERODIVIDE)
		abstr |= _EM_ZERODIVIDE;
	if (cw & IEM_OVERFLOW)
		abstr |= _EM_OVERFLOW;
	if (cw & IEM_UNDERFLOW)
		abstr |= _EM_UNDERFLOW;
	if (cw & IEM_INEXACT)
		abstr |= _EM_INEXACT;
	if (cw & IEM_DENORMAL)
		abstr |= _EM_DENORMAL;

	//
	// Set rounding mode
	//

	switch (cw & IMCW_RC) {
	case IRC_NEAR:
		abstr |= _RC_NEAR;
		break;
	case IRC_UP:
		abstr |= _RC_UP;
		break;
	case IRC_DOWN:
		abstr |= _RC_DOWN;
		break;
	case IRC_CHOP:
		abstr |= _RC_CHOP;
		break;
	}

	//
	// Set Precision mode
	//

	switch (cw & IMCW_PC) {
	case IPC_64:
		abstr |= _PC_64;
		break;
	case IPC_53:
		abstr |= _PC_53;
		break;
	case IPC_24:
		abstr |= _PC_24;
		break;
	}


	//
	// Infinity control (bit can be programmed but has no effect)
	//

	if (cw & IMCW_IC) {
		abstr |= _IC_AFFINE;
	}

	return abstr;

}

uint32_t __get_abstract_status_x87(uint16_t sw) {
	unsigned int abstr = 0;
	
	if (sw & ISW_INVALID)
		abstr |= _SW_INVALID;
	if (sw & ISW_ZERODIVIDE)
		abstr |= _SW_ZERODIVIDE;
	if (sw & ISW_OVERFLOW)
		abstr |= _SW_OVERFLOW;
	if (sw & ISW_UNDERFLOW)
		abstr |= _SW_UNDERFLOW;
	if (sw & ISW_INEXACT)
		abstr |= _SW_INEXACT;
	if (sw & ISW_DENORMAL)
		abstr |= _SW_DENORMAL;

	return abstr;
}

uint32_t __cdecl _getfpcontrolword() {
#ifdef _M_IX86
	uint16_t cw;
	__asm fnstcw cw;
#else
	uint16_t cw = _ctrlfp(0, 0);
#endif
	return __get_abstract_control_x87(cw) & 0x31F;		//!!!?
}

uint32_t __cdecl _getfpstatusword() {
#ifdef _M_IX86
	uint16_t sw;
	__asm fnstsw sw;
#else
	uint16_t sw = _statfp();
#endif
	return  __get_abstract_control_x87(sw) & 0x1F;		//!!!?
}

struct fenv_x87 {
	uint16_t ControlWord, pad1;
	uint16_t StatusWord, pad2;
	uint16_t TagWord, pad3;
	uint32_t  InstrP;
	uint16_t InstrSel, pad4;
	uint32_t  DataP;
	uint16_t DataSel, pad5;
};

void __cdecl _setfpcontrolword(uint32_t v) {
#ifdef _M_IX86
	struct fenv_x87 env;
	__asm fnstenv env;
	env.ControlWord = env.ControlWord & ~__get_machine_control(_MCW_RC | _EM_INEXACT | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID) | __get_machine_control(v);
	__asm fldenv env;
#else
	_ctrlfp(__get_machine_control(v), 0xFFFF);
#endif
}

static_assert(sizeof(struct fenv_x87)==28, "Invalid fenv_x87 size");


uint16_t __fastcall _get_fpsr();
void __fastcall _set_fpsr(uint16_t v);

void __cdecl _setfpstatusword(uint32_t v) {
#ifdef _M_IX86
	struct fenv_x87 env;
	__asm fnstenv env;
	env.StatusWord = env.StatusWord & ~__get_machine_status(FE_ALL_EXCEPT) | __get_machine_status((uint8_t)v);
	__asm fldenv env;
#else
	_set_fpsr(_get_fpsr() & ~__get_machine_status(FE_ALL_EXCEPT) | __get_machine_status((uint8_t)v));
#endif
}



