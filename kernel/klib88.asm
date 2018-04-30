; This file contains a number of assembly code utility routines	needed by the
; kernel.  They	are:
;
;   phys_copy:	copies data from anywhere to anywhere in memory
;   cp_mess:	copies messages	from source to destination
;   port_out:	outputs	data on	an I/O port
;   port_in:	inputs data from an I/O	port
;   lock:	disable	interrupts
;   unlock:	enable interrupts
;   restore:	restore	interrupts [enable/disabled] as	they were before lock[]
;   build_sig:	build 4	word structure pushed onto stack for signals
;   csv:	procedure prolog to save the registers
;   cret:	procedure epilog to restore the	registers
;   get_chrome:	returns	0 is display is	monochrome, 1 if it is color
;   vid_copy:	copy data to video ram [on color display during	retrace	only]
;   get_byte:	reads a	byte from a user program and returns it	as value

; The following	procedures are defined in this file and	called from outside it.
global _phys_copy, _cp_mess, _port_out, _port_in, _lock, _unlock, _restore
global _build_sig, _get_chrome, _vid_copy, _get_byte
global _reboot, _wreboot, _portw_in, _portw_out  
global _port_read, _port_write

; The following	external procedure is called in	this file.
extern _panic

; Variables and	data structures
extern _color, _cur_proc, _proc_ptr, _splimit, _vid_mask
global _vec_table

%include "../h/model.h"

segment _TEXT
;===========================================================================
;				phys_copy
;===========================================================================
; This routine copies a	block of physical memory.  It is called	by:
;    phys_copy(	(long) source, (long) destination, (long) bytecount)

_phys_copy:
	pushf			; save flags
	cli			; disable interrupts
	push bp			; save the registers
	push ax			; save ax
	push bx			; save bx
	push cx			; save cx
	push dx			; save dx
	push si			; save si
	push di			; save di
	push ds			; save ds
	push es			; save es
	mov bp,sp		; set bp to point to saved es

  L0:	mov ax,[bp+28]		; ax = high-order word of 32-bit destination
	mov di,[bp+26]		; di = low-order word of 32-bit	destination
	mov cx,4		; start	extracting click number	from dest
  L1:	rcr ax,1		; click	number is destination address /	16
	rcr di,1		; it is	used in	segment	register for copy
	loop L1			; 4 bits of high-order word are	used
	mov es,di		; es = destination click

	mov ax,[bp+24]		; ax = high-order word of 32-bit source
	mov si,[bp+22]		; si = low-order word of 32-bit	source
	mov cx,4		; start	extracting click number	from source
  L2:	rcr ax,1		; click	number is source address / 16
	rcr si,1		; it is	used in	segment	register for copy
	loop L2			; 4 bits of high-order word are	used
	mov ds,si		; ds = source click

	mov di,[bp+26]		; di = low-order word of dest address
	and di,000Fh		; di = offset from paragraph  in es
	mov si,[bp+22]		; si = low-order word of source	address
	and si,000Fh		; si = offset from paragraph  in ds

	mov dx,[bp+32]		; dx = high-order word of byte count
	mov cx,[bp+30]		; cx = low-order word of byte count

	test cx,8000h		; if bytes >= 32768, only do 32768
	jnz L3			; per iteration
	test dx,0FFFFh		; check	high-order 17 bits to see if bytes
	jnz L3			; if bytes >= 32768 then go to L3
	jmp short L4		; if bytes < 32768 then	go to L4
  L3:	mov cx,8000h		; 0x8000 is unsigned 32768
  L4:	mov ax,cx		; save actual count used in ax;	needed later

	test cx,0001h		; should we copy a byte	or a word at a time?
	jz L5			; jump if even
	rep movsb		; copy 1 byte at a time
	jmp short L6		; check	for more bytes

  L5:	shr cx,1		; word copy
	rep movsw		; copy 1 word at a time

  L6:	mov dx,[bp+32]		; decr count, incr src & dst, iterate if needed
	mov cx,[bp+30]		; dx ||	cx is 32-bit byte count
	xor bx,bx		; bx ||	ax is 32-bit actual count used
	sub cx,ax		; compute bytes	- actual count
	sbb dx,bx		; dx ;;	cx is  bytes not yet processed
	or cx,cx		; see if it is 0
	jnz L7			; if more bytes	then go	to L7
	or dx,dx		; keep testing
	jnz L7			; if loop done,	fall through

	pop es			; restore all the saved	registers
	pop ds			; restore ds
	pop di			; restore di
	pop si			; restore si
	pop dx			; restore dx
	pop cx			; restore cx
	pop bx			; restore bx
	pop ax			; restore ax
	pop bp			; restore bp
	popf			; restore flags
	ret			; return to caller

L7:	mov [bp+32],dx		; store	decremented byte count back in mem
	mov [bp+30],cx		; as a long
	add [bp+26],ax		; increment destination
	adc [bp+28],bx		; carry	from low-order word
	add [bp+22],ax		; increment source
	adc [bp+24],bx		; carry	from low-order word
	jmp L0			; start	next iteration


;===========================================================================
;				cp_mess
;===========================================================================
; This routine is makes	a fast copy of a message from anywhere in the address
; space	to anywhere else.  It also copies the source address provided as a
; parameter to the call	into the first word of the destination message.
; It is	called by:
;    cp_mess[src, src_clicks, src_offset, dst_clicks, dst_offset]
; where	all 5 parameters are shorts [16-bits].
;
; Note that the	message	size, 'Msize' is in WORDS [not bytes] and must be set
; correctly.  Changing the definition of message the type file and not changing
; it here will lead to total disaster.
; This routine destroys	ax.  It	preserves the other registers.

Msize equ	12              ; size of a message in 16-bit words
_cp_mess:
	push bp			; save bp
	push es			; save es
	push ds			; save ds
	mov bp,sp		; index	off bp because machine can't use sp
	pushf			; save flags
	cli			; disable interrupts
	push cx			; save cx
	push si			; save si
	push di			; save di

	mov ax,[bp+8]		; ax = process number of sender
	mov di,[bp+16]		; di = offset of destination buffer
	mov es,[bp+14]		; es = clicks of destination
	mov si,[bp+12]		; si = offset of source	message
	mov ds,[bp+10]		; ds = clicks of source	message
	mov [es:di],ax		; copy sender's	process	number to dest message
	add si,2		; don't	copy first word
	add di,2		; don't	copy first word
	mov cx,Msize-1		; remember, first word doesn't count
	rep movsw		; iterate cx times to copy the message

	pop di			; restore di
	pop si			; restore si
	pop cx			; restore cs
	popf			; restore flags
	pop ds			; restore ds
	pop es			; restore es
	pop bp			; restore bp
	ret			; that's all folks!


;===========================================================================
;				port_out
;===========================================================================
; port_out(port, value)	writes 'value' on the I/O port 'port'.

_port_out:
	push bx			; save bx
	mov bx,sp		; index	off bx
	push ax			; save ax
	push dx			; save dx
	mov dx,[bx+4]		; dx = port
	mov ax,[bx+6]		; ax = value
	out dx,al		; output 1 byte
	pop dx			; restore dx
	pop ax			; restore ax
	pop bx			; restore bx
	ret			; return to caller


;===========================================================================
;				port_in
;===========================================================================
; port_in(port,	&value)	reads from port	'port' and puts	the result in 'value'.

_port_in:
	push bx			; save bx
	mov bx,sp		; index	off bx
	push ax			; save ax
	push dx			; save dx
	mov dx,[bx+4]		; dx = port
	in  al,dx		; input	1 byte
	xor ah,ah		; clear	ah
	mov bx,[bx+6]		; fetch	address	where byte is to go
	mov [bx],ax		; return byte to caller	in param
	pop dx			; restore dx
	pop ax			; restore ax
	pop bx			; restore bx
	ret			; return to caller


;*===========================================================================*
;*				portw_out				     *
;*===========================================================================*
; portw_out(port, value) writes 'value' on the I/O port 'port'.

_portw_out:
	push bx			; save bx
	mov bx,sp		; index off bx
	push ax			; save ax
	push dx			; save dx
	mov dx,[4+bx]		; dx = port
	mov ax,[6+bx]		; ax = value
	out dx,ax               ; output 1 word
	pop dx			; restore dx
	pop ax			; restore ax
	pop bx			; restore bx
	ret			; return to caller


;*===========================================================================*
;*				portw_in				     *
;*===========================================================================*
; portw_in(port, &value) reads from port 'port' and puts the result in 'value'.

_portw_in:
	push bx			; save bx
	mov bx,sp		; index off bx
	push ax			; save ax
	push dx			; save dx
	mov dx,[4+bx]		; dx = port
	in  ax,dx               ; input 1 word
	mov bx,[6+bx]		; fetch address where byte is to go
	mov [bx],ax		; return byte to caller in param
	pop dx			; restore dx
	pop ax			; restore ax
	pop bx			; restore bx
	ret			; return to caller
        

;*===========================================================================*
;*				port_read				     *
;*===========================================================================*
; PUBLIC void port_read(port_t port, phys_bytes destination,unsigned bytcount);
; Transfer data from (hard disk controller) port to memory.

OFF_MASK        equ     0x000F
HCHIGH_MASK     equ	0x0F
HCLOW_MASK      equ	0xF0
HCLICK_SHIFT 	equ	4

_port_read:
	push bp
	mov bp,sp
	push cx
	push dx
	push di
	push es
	mov ax,[4+2+bp]         ; destination addr in dx:ax
	mov dx,[4+2+2+bp]
	mov di,ax               ; di = dest offset = address % 16
	and di,OFF_MASK
	and dl,HCHIGH_MASK	; es = dest segment = address / 16 % 0x10000
	and al,HCLOW_MASK
	or al,dl
	mov cl,HCLICK_SHIFT
	ror ax,cl
	mov es,ax

	mov cx,[4+2+4+bp]	; count in bytes
	shr cx,1                ; count in words
	mov dx,[4+bp]           ; port to read from
	rep insw
	pop es
	pop di
	pop dx
	pop cx
	mov sp,bp
	pop bp
	ret


;*===========================================================================*
;*				port_write				     *
;*===========================================================================*
; PUBLIC void port_write(port_t port, phys_bytes source, unsigned bytcount);
; Transfer data from memory to (hard disk controller) port.

_port_write:
	push bp
	mov	bp,sp
	push cx
	push dx
	push si
	push ds
	mov ax,[4+2+bp]         ; source addr in dx:ax
	mov dx,[4+2+2+bp]
	mov si,ax               ; si = source offset = address % 16
	and si,OFF_MASK
	and dl,HCHIGH_MASK	; ds = source segment = address / 16 % 0x10000
	and al,HCLOW_MASK
	or  al,dl
	mov cl,HCLICK_SHIFT
	ror ax,cl
	mov ds,ax
	mov cx,[4+2+4+bp]	; count in bytes
	shr cx,1                ; count in words
	mov dx,[4+bp]           ; port to read from
	rep outsw
	pop ds
	pop si
	pop dx
	pop cx
	mov sp,bp
	pop bp
	ret        
        
 
;===========================================================================
;				lock
;===========================================================================
; Disable CPU interrupts.

_lock:
	pushf			; save flags on	stack
	cli			; disable interrupts
	pop word[lockvar]       ; save flags for possible restoration later
	ret			; return to caller


;===========================================================================
;				unlock
;===========================================================================
; Enable CPU interrupts.

_unlock:
	sti			; enable interrupts
	ret			; return to caller


;===========================================================================
;				restore
;===========================================================================
; Restore enable/disable bit to	the value it had before	last lock.

_restore:
	push word[lockvar]      ; push flags as	they were before previous lock
	popf			; restore flags
	ret			; return to caller


;===========================================================================
;				build_sig
;===========================================================================
; Build	a structure that is pushed onto	the stack for signals.	It contains
; pc, psw, etc., and is	machine	dependent. The format is the same as generated
; by hardware interrupts, except that after the	"interrupt", the signal	number
; is also pushed.  The signal processing routine within	the user space first
; pops the signal number, to see which function	to call.  Then it calls	the
; function.  Finally, when the function	returns	to the low-level signal
; handling routine, control is passed back to where it was prior to the	signal
; by executing a return-from-interrupt instruction, hence the need for using
; the hardware generated interrupt format on the stack.	 The call is:
;     build_sig(sig_stuff, rp, sig)

; Offsets within proc table
PC    equ	24
csreg equ	18
PSW   equ	28

_build_sig:
	push bp			; save bp
	mov bp,sp		; set bp to sp for accessing params
	push bx			; save bx
	push si			; save si
	mov bx,[bp+4]		; bx points to sig_stuff
	mov si,[bp+6]		; si points to proc table entry
	mov ax,[bp+8]		; ax = signal number
	mov [bx],ax		; put signal number in sig_stuff
	mov ax,[si+PC]		; ax = signalled process' PC
	mov [bx+2],ax		; put pc in sig_stuff
	mov ax,[si+csreg]	; ax = signalled process' cs
	mov [bx+4],ax		; put cs in sig_stuff
	mov ax,[si+PSW]		; ax = signalled process' PSW
	mov [bx+6],ax		; put psw in sig_stuff
	pop si			; restore si
	pop bx			; restore bx
	pop bp			; restore bp
	ret			; return to caller


;===========================================================================
;		csv & cret  (compiler generated	symbols)
;===========================================================================
; This version of csv replaces the standard one.  It checks for stack overflow
; within the kernel in a simpler way than is usually done. cret is standard.

csv:
	pop bx			; bx = return address
	push bp			; stack	old frame pointer
	mov bp,sp		; set new frame	pointer	to sp
	push di			; save di
	push si			; save si
	sub sp,ax		; ax =	bytes of local variables
	cmp sp,[_splimit]	; has kernel stack grown too large
	jbe csv1		; if sp	is too low, panic
	jmp [bx]		; normal return: copy bx to program counter

csv1:
	mov word[_splimit],0    ; prevent call to panic	from aborting in csv
	mov bx,[_proc_ptr]      ; update rp->p_splimit
	mov WORD[bx+50],0       ; rp->sp_limit = 0
	push word[_cur_proc]    ; task number
	mov ax,stkoverrun       ; stack	overran	the kernel stack area
	push ax                 ; push first parameter
	call _panic		; call is: panic(stkoverrun, cur_proc)
	jmp csv1		; this should not be necessary

        
cret:
	lea	sp,[bp-4]	; set sp to point to saved si
	pop	si		; restore saved	si
	pop	di		; restore saved	di
	pop	bp		; restore bp
	ret			; end of procedure

;===========================================================================
;				get_chrome
;===========================================================================
; This routine calls the BIOS to find out if the display is monochrome or
; color.  The drivers are different, as	are the	video ram addresses, so	we
; need to know.

_get_chrome:
	int 11h			; call the BIOS	to get equipment type
	and al,30h		; isolate color/mono field
	cmp al,30h		; 0x30 is monochrome
	je getchr1		; if monochrome	then go	to getchr1
	mov ax,1		; color	= 1
	ret			; color	return
getchr1:xor ax,ax		; mono = 0
	ret			; monochrome return


;===========================================================================
;				vid_copy
;===========================================================================
; This routine takes a string of [character, attribute]	pairs and writes them
; onto the screen.  For	a color	display, the writing only takes	places during
; the vertical retrace interval, to avoid displaying garbage on	the screen.
; The call is:
;     vid_copy(buffer, videobase, offset, words)
; where
;     'buffer'	  is a pointer to the (character, attribute) pairs
;     'videobase' is 0xB800 for	color and 0xB000 for monochrome	displays
;     'offset'	  tells	where within video ram to copy the data
;     'words'	  tells	how many words to copy
; if buffer is zero, the fill character	(BLANK)	is used

BLANK equ	0700h

_vid_copy:
	push bp			; we need bp to	access the parameters
	mov bp,sp		; set bp to sp for indexing
	push si			; save the registers
	push di			; save di
	push bx			; save bx
	push cx			; save cx
	push dx			; save dx
	push es			; save es
vid0:	mov si,[bp+4]		; si = pointer to data to be copied
	mov di,[bp+8]		; di = offset within video ram
	and di,[_vid_mask]	; only 4K or 16K counts
	mov cx,[bp+10]		; cx = word count for copy loop
	mov dx,03DAh		; prepare to see if color display is retracing

	mov bx,di		; see if copy will run off end of video ram
	add bx,cx		; compute where copy ends
	add bx,cx		; bx = last character copied + 1
	sub bx,[_vid_mask]	; bx = # characters beyond end of video ram
	sub bx,1		; note: dec bx doesn't set flags properly
	jle vid1		; jump if no overrun
	sar bx,1		; bx = # words that don't fit in video ram
	sub cx,bx		; reduce count by overrun
	mov [tmp],cx		; save actual count used for later

vid1:	test word[_color],1	; skip vertical	retrace	test if	display	is mono
	jz vid4			; if monochrome	then go	to vid.2

;  vid2:in			; with a color display,	you can	only copy to
;	test al,010q		; the video ram	during vertical	retrace, so
;	jnz vid2		; wait for start of retrace period.  Bit 3 of
vid3:   in al,dx		; 0x3DA	is set during retrace.	First wait
	test al,010q		; until	it is off (no retrace),	then wait
	jz vid3			; until	it comes on (start of retrace)

vid4:   pushf			; copying may now start; save the flags
	cli			; interrupts just get in the way: disable them
	mov es,[bp+6]		; load es now: int routines may	ruin it

	cmp si,0		; si = 0 means blank the screen
	je vid7			; jmp for blanking
;	lock nop		; this is a trick for the IBM PC-simulator only
				; 'lock' indicates a video ram access
	rep movsw		; this is the copy loop

vid5:   popf			; restore flags
	cmp bx,0		; if bx < 0, then no overrun and we are done
	jle vid6		; jump if everything fit
	mov [bp+10],bx		; set up residual count
	mov word[bp+8],0        ; JER added size: start copying at base of video ram
	cmp word[bp+4],0        ; JER added size: NIL_PTR means store blanks
	je vid0			; go do it
	mov si,[tmp]		; si = count of words copied
	add si,si		; si = count of bytes copied
	add [bp+4],si		; increment buffer pointer
	jmp vid0		; go copy some more

vid6:				; JER - not in original source (vid6 label) - hope it's right
	pop es			; restore registers
	pop dx			; restore dx
	pop cx			; restore cx
	pop bx			; restore bx
	pop di			; restore di
	pop si			; restore si
	pop bp			; restore bp
	ret			; return to caller

vid7:   mov ax,BLANK		; ax = blanking	character
	rep stosw		; blank	the screen
	jmp vid5		; done

;===========================================================================
;				get_byte
;===========================================================================
; This routine is used to fetch	a byte from anywhere in	memory.
; The call is:
;     c	= get_byte(seg,	off)
; where
;     'seg' is the value to put	in es
;     'off' is the offset from the es value

_get_byte:
	push bp			; save bp
	mov bp,sp		; we need to access parameters
	push es			; save es
	mov es,[bp+4]		; load es with segment value
	mov bx,[bp+6]		; load bx with offset from segment
	mov al,[es:bx]		; al = byte
	xor ah,ah		; ax = byte
	pop es			; restore es
	pop bp			; restore bp
	ret			; return to caller


;===========================================================================
;				reboot & dump
;===========================================================================
; This code reboots the	PC

_reboot:
	cli			; disable interrupts
	mov al,20h
	out 20h,al		; re-enable interrupt controller
	call _resvec		; restore the vectors in low core
	int 19h			; reboot the PC

_wreboot:
	cli			; disable interrupts
	mov al,20h		; re-enable interrupt controller
	out 20h,al
	call _resvec		; restore the vectors in low core
	xor ax,ax		; wait for character before continuing
	int 16h			; get char
	int 19h			; reboot the PC
        
; Restore the interrupt	vectors	in low core.
_resvec:

	cld
	mov cx,2*65
	mov si,_vec_table
	xor di,di
	mov es,di
    rep movsw
	ret

        
segment _DATABEG

segment _DATA
lockvar:        DW      0       ; place	to store flags for lock()/restore()
tmp:            DW      0       ; count of bytes already copied
 _vec_table:    times   130 DW 0; storage for interrupt	vectors
stkoverrun:     DB      "Kernel stack overrun,	task = ",0          
segment _DATAEND

segment _BSS

segment _BSSEND
;_end:

segment STACK
