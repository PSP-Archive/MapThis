	.set noreorder

#include "pspstub.s"

	STUB_START "sioprx",0x40090000,0x00020005
	STUB_FUNC  0x8128D0E0,sioInit
	STUB_FUNC  0xBBDD24FB,sioReadChar
	STUB_END
