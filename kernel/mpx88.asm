; This file is part of the lowest layer	of the Minix kernel.  All process
; switching and	message	handling is done here and in file "proc.c".  This file
; is entered on	every transition to the	kernel,	both for sending/receiving
; messages and for all interrupts.  In all cases, the trap or interrupt
; routine first	calls save() to	store the machine state	in the proc table.
; Then the stack is switched to	k_stack.  Finally, the real _trap or interrupt
; handler (in C) is called.  When it returns, the interrupt routine jumps to
; restart, to run the process or task whose number is in 'cur_proc'.
;
; The external entry points into this file are:
;   s_call:	process	or task	wants to send or receive a message
;   tty_int:	interrupt routine for each key depression and release
;   lpr_int:	interrupt routine for each line	printer	interrupt
;   disk_int:	disk interrupt routine
;   wini_int:	winchester interrupt routine
;   clock_int:	clock interrupt	routine	[HZ times per second]
;   surprise:	all other interrupts < 16 are vectored here
;   trp:	all traps with vector >= 16 are	vectored here
;   divide:	divide overflow traps are vectored here
;   restart:	start running a	task or	process

; include the following	from const.h:
K_STACK_BYTES	EQU  256
IDLE		EQU -999

; include the following	from ../h/com.h
DISKINT		EQU  1
CLOCK		EQU -3
CLOCK_TICK	EQU  2
FLOPPY		EQU -5
WINI		EQU -6

; The following	procedures are defined in this file and	called from outside it.
global $main, _tty_int, _lpr_int, _clock_int, _disk_int, _wini_int
global _s_call, _surprise, _trp, _divide, _restart

; The following	external procedures are	called in this file.
extern _main, _sys_call, _interrupt, _keyboard
extern _panic, _unexpected_int, _trap, _div_trap, _pr_char

; Variables and	data structures.
global _sizes, _brksize, _splimit, _end
extern _cur_proc, _proc_ptr, _int_mess
extern _scan_code, _k_stack

; The following	constants are offsets into the proc table.
esreg equ	14
dsreg equ	16
csreg equ	18
ssreg equ	20
spreg equ	22
PC    equ	24
PSW   equ	28
SPLIM equ	50
OFF   equ	18
ROFF  equ	12

%include "../h/model.h"

segment _TEXT
;===========================================================================
;				Minix
;===========================================================================
$main:
Minix:				; this is the entry point for the Minix	kernel.
	jmp short M0		; skip over the	next word(s)
	nop			; build	writes the ds value at text address 4
        nop
ker_ds:	DW  DGROUP		; this word will contain kernel's ds value
				; and it forces	relocation for dos2out
     M0:cli			; disable interrupts
	mov ax,cs		; set up segment registers
	mov ds,ax		; set up ds
	mov ax,[cs:ker_ds]	; build	has loaded this	word with ds value
	mov ds,ax		; ds now contains proper value
	mov ss,ax		; ss now contains proper value
	mov [_scan_code],bx     ; save scan code for '=' key from bootstrap
	mov sp,_k_stack	        ; set sp to point to the top of	the
	add sp,K_STACK_BYTES    ; kernel stack
        
;CR0_MP equ 010b
;CR0_EM equ 0100b
;CR0_TS equ 01000b
;        MOV EDX, CR0            ; Start probe, get CR0
;        AND EDX, (-1) - (CR0_MP+CR0_EM+CR0_TS); clear TS and EM to force fpu access
;        MOV CR0, EDX            ; store control word
;        FNINIT
        fninit

	call _main		; start	the main program of Minix

     M1:jmp M1			; this should never be executed


;===========================================================================
;				s_call
;===========================================================================
_s_call:                        ; System calls are vectored here.
	call save		; save the machine state
	mov bp,[_proc_ptr]	; use bp to access sys call parameters
	push word[bp+2]		; push(pointer to user message)	(was bx)
	push word[bp]		; push(src/dest) (was ax)
	push word[_cur_proc] 	; push caller
	push word[bp+4]		; push(SEND/RECEIVE/BOTH) (was cx)
	call _sys_call		; sys_call(function, caller, src_dest, m_ptr)
	jmp _restart		; jump to code to restart proc/task running


;===========================================================================
;				tty_int
;===========================================================================
_tty_int:			; Interrupt routine for	terminal input.
	call save		; save the machine state
	call _keyboard		; process a _keyboard interrupt
	jmp  _restart		; continue execution


;===========================================================================
;				lpr_int
;===========================================================================
_lpr_int:			; Interrupt routine for	terminal input.
	call save		; save the machine state
	call _pr_char		; process a line printer interrupt
	jmp  _restart		; continue execution


;===========================================================================
;				disk_int
;===========================================================================
_disk_int:			; Interrupt routine for	the floppy disk.
	call save               ; save the machine state
	mov  word[_int_mess+2],DISKINT          ; build	message	for disk task
	mov  ax,_int_mess	; prepare to call interrupt[FLOPPY, &intmess]
	push ax                 ; push second parameter
	mov  ax,FLOPPY		; prepare to push first	parameter
	push ax			; push first parameter
	call _interrupt		; this is the call
	jmp  _restart		; continue execution

;*===========================================================================*
;*				wini_int				     *
;*===========================================================================*
_wini_int:			; Interrupt routine for the winchester disk.
	call save		; save the machine state
	mov  word[_int_mess+2],DISKINT   ; build message for winchester task
	mov  ax,_int_mess	; prepare to call interrupt(WINI, &intmess)
	push ax			; push second parameter
	mov  ax,WINI		; prepare to push first parameter
	push ax			; push first parameter
	call _interrupt		; this is the call
	jmp  _restart		; continue execution    

;===========================================================================
;				clock_int
;===========================================================================
_clock_int:			; Interrupt routine for	the clock.
	call save               ; save the machine state
	mov word[_int_mess+2],CLOCK_TICK        ; build message for clock task
	mov ax,_int_mess	; prepare to call interrupt(CLOCK,&intmess)
	push ax                 ; push second parameter
	mov  ax,CLOCK		; prepare to push first	parameter
	push ax			; push first parameter
	call _interrupt		; this is the call
	jmp  _restart		; continue execution


;===========================================================================
;				surprise
;===========================================================================
_surprise:			; This is where	unexpected interrupts come.
	call save		; save the machine state
	call _unexpected_int    ; go _panic
	jmp  _restart		; never	executed


;===========================================================================
;				trp
;===========================================================================
_trp:				; This is where	unexpected traps come.
	call save		; save the machine state
	call _trap		; print	a message
	jmp _restart		; this error is	not fatal


;===========================================================================
;				divide					     
;===========================================================================
_divide:                        ; This is where divide overflow traps come.
	call save		; save the machine state
	call _div_trap		; print a message
	jmp _restart		; this error is not fatal


;===========================================================================
;				save
;===========================================================================
save:				; save the machine state in the	proc table.
	push ds			; stack: psw/cs/pc/ret addr/ds
	push cs			; prepare to restore ds
	pop ds			; ds has now been set to cs
	mov ds,[ker_ds]		; word 4 in kernel text	space contains ds value
	pop word[ds_save]       ; stack: psw/cs/pc/ret addr
	pop word[ret_save]      ; stack: psw/cs/pc
	mov [bx_save],bx        ; save bx for later ; we need a	free register
	mov bx,[_proc_ptr]	; start	save set up; make bx point to save area
	add bx,OFF		; bx points to place to	store cs
	pop word[bx+PC-OFF]     ; store	pc in proc table
	pop word[bx+csreg-OFF]	; store	cs in proc table
	pop word[bx+PSW-OFF]    ; store	psw
	mov [bx+ssreg-OFF],ss	; store	ss
	mov [bx+spreg-OFF],sp	; sp as	it was prior to	interrupt
	mov sp,bx		; now use sp to	point into proc	table/task save
	mov bx,ds		; about	to set ss
	mov ss,bx		; set ss
	push word[ds_save]      ; start	saving all the registers, sp first
	push es			; save es between sp and bp
	mov es,bx		; es now references kernel memory too
	push bp			; save bp
	push di			; save di
	push si			; save si
	push dx			; save dx
	push cx			; save cx
	push word[bx_save]      ; save original	bx
	push ax                 ; all registers now saved
	mov sp,_k_stack         ; temporary stack for interrupts
	add sp,K_STACK_BYTES    ; set sp to top of temporary stack
	mov word[_splimit],_k_stack     ; limit for temporary stack
        add ax,8
	mov [_splimit],ax       ; splimit checks for stack overflow
	mov ax,[ret_save]       ; ax = address to return to
	jmp ax			; return to caller; Note: sp points to saved ax


;===========================================================================
;				restart
;===========================================================================
_restart:			; This routine sets up and runs	a proc or task.
	cmp word[_cur_proc],IDLE; restart user; if cur_proc = IDLE, go idle
	je _idle		; no user is runnable, jump to idle routine
	cli			; disable interrupts
	mov sp,[_proc_ptr]	; return to user, fetch	regs from proc table
	pop ax			; start	restoring registers
	pop bx			; restore bx
	pop cx			; restore cx
	pop dx			; restore dx
	pop si			; restore si
	pop di			; restore di
	mov [lds_low],bx        ; lds_low contains bx
	mov bx,sp		; bx points to saved bp	register
	mov bp,[bx+SPLIM-ROFF]	; splimit = p_splimit
	mov [_splimit],bp       ; ditto
	mov bp,[bx+dsreg-ROFF]	; bp = ds
	mov [lds_low+2],bp	; lds_low+2 contains ds
	pop bp			; restore bp
	pop es			; restore es
	mov sp,[bx+spreg-ROFF]	; restore sp
	mov ss,[bx+ssreg-ROFF]	; restore ss using the value of	ds
	push word[bx+PSW-ROFF]	; push psw (flags)
	push word[bx+csreg-ROFF]; push cs
	push word[bx+PC-ROFF]	; push pc
	lds  bx,[DWORD lds_low] ; restore ds and bx in one fell swoop
	iret			; return to user or task

        
;===========================================================================
;				idle
;===========================================================================
_idle:				; executed when	there is no work
        sti			; enable interrupts
L3:	wait			; just idle while waiting for interrupt
        jmp L3			; loop until interrupt

        
;===========================================================================
;				data
;===========================================================================      
segment _DATABEG                ; DATABEG ensures it is the beginning of all data
_sizes:	  DW	526Fh           ; this must be the first data entry (magic nr)
	  times 7 dw 0          ; space	for build table	- total	16b
_splimit: DW	0               ; stack	limit for current task (kernel only)
bx_save:  DW	0               ; storage for bx
ds_save:  DW	0               ; storage for ds
ret_save: DW	0               ; storage for return address
lds_low:  DW	0,0             ; storage used for restoring ds:bx
_brksize: DW	_end+2          ; first	free memory in kernel
ttyomess: DB	"RS232 interrupt",0
segment _DATA

segment _DATAEND

segment _BSS

segment _BSSEND                 ; BSSEND holds nothing. The label in
_end:                           ; the segment just tells us where
                                ; the data+bss ends.
                                
segment STACK                   ; Satisfy DOS-linker (dummy stack)
