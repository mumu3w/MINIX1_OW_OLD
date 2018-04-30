
extern _main, _stackpt
global @main, _data_org, _brksize, _sp_limit, _end

%include "../h/model.h"

segment _TEXT
@main:	jmp	short L0
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
	;ORG 10h                ; kernel uses this area	as stack for inital IRET
     L0:mov	sp,[_stackpt]
	call	_main
     L1:jmp L1			; this will never be executed
	mov	ax,DGROUP	; force	relocation for dos2out (never executed)
        
segment _DATABEG                ; fs needs to know where build stuffed table
_data_org: DW    0DADAh		; 0xDADA is magic number for build
           times 7 DW 0		; first 8 words of MM, FS, INIT are for stack
_brksize:  DW    _end           ; first free memory
_sp_limit: DW    0

segment _DATA

segment _DATAEND

segment _BSS

segment _BSSEND                 ; _BSSEND holds nothing. The label in
_end:                           ; the segment just tells us where
                                ; the data+bss ends.

segment STACK                   ; Add stack to satisfy DOS-linker
