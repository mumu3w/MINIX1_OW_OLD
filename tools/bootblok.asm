; page ,132
; When the PC is powered on, it reads the first block from the floppy
; disk into address 0x7C00 and jumps to it.  This boot block must contain
; the boot program in this file.  The boot program first copies itself to
; address 192K - 512 (to get itself out of the way).  Then it loads the 
; operating system from the boot diskette into memory, and then jumps to fsck.
; Loading is not trivial because the PC is unable to read a track into
; memory across a 64K boundary, so the positioning of everything is critical.
; The number of sectors to load is contained at address 504 of this block.
; The value is put there by the build program after it has discovered how
; big the operating system is.  When the bootblok program is finished loading,
; it jumps indirectly to the program (fsck) which address is given by the
; last two words in the boot block. 
;
; Summary of the words patched into the boot block by build:
; Word at 502: # sectors to load
; Word at 504: # DS value for fsck
; Word at 506: # PC value for fsck
; Word at 508: # CS value for fsck
; Word at 510: # 0xaa55
;
; This version of the boot block must be assembled without separate I & D
; space.  

        LOADSEG equ 0060h         ; here the boot block will start loading
        BIOSSEG equ 07C0h         ; here the boot block itself is loaded
        BOOTSEG equ 2FE0h         ; here it will copy itself (192K-512b)
        DSKBASE equ 120           ; 120 = 4 * 0x1E = ptr to disk parameters
;        JMPI    = 0EAh           ; opcode for jmp inter-segment (far jmp)
cpu 8086
bits 16
;CODE    SEGMENT
;        ASSUME cs:CODE,ds:CODE

;-------------------------------+
;          bootblok             ;
;-------------------------------+
;
; copy bootblock to bootseg
        mov     ax,BIOSSEG
        mov     ds,ax
        xor     si,si           ; ds:si - original block
        mov     ax,BOOTSEG
        mov     es,ax
        xor     di,di           ; es:di - new block
        mov     cx,256          ; #  words to move
        rep     movsw           ; copy loop

; start boot procedure
;       jmpi    start,BOOTSEG   ; set cs to bootseg

;        DB      JMPI            ; normal MASM gets too complicated for this.
;        DW      OFFSET start    ;  Use a trick and pre-code the
;        DW      BOOTSEG         ;  instructions and addresses.
        jmp BOOTSEG:start

start:                          ; start < 16
        mov     dx,cs
        mov     ds,dx           ; set ds to cs
        xor     ax,ax
        mov     es,ax           ; set es to 0
        mov     ss,dx           ; set ss to cs i.e., stack in high core
        mov     sp,1536         ; initialize sp to high core
%if 0
; initialize disk parameters
	mov	ax, atpar
	mov	[es:DSKBASE],ax	; tentatively assume 1.2M diskette
	mov	[es:DSKBASE+2],dx
%endif
; print greeting
	mov	ax,2		; reset video
	int 	10h
        mov     ax,0200h        ; BIOS call to move cursor to ul corner
        xor     bx,bx
        xor     dx,dx
        int     10h
        mov     bx, greet
        call    print
%if 0
; Determine if this is a 1.2M diskette by trying to read sector 15.
	xor	ax,ax
	int	13h
	xor	ax,ax
	mov	es,ax
	mov	ax,0201h
	mov	bx,0600h
	mov	cx,000fh
	mov	dx,0000h
	int	13h
	jnb	L1

; Error.  It wasn't 1.2M.  Now set up for 360K.
	mov	WORD[tracksiz],9	; 360K uses 9 sectors/track
	xor	ax,ax
	mov	es,ax
	mov	ax, pcpar
	mov	[es:DSKBASE],ax
	int	13h		; diskette reset
%endif
L1:

; Load the operating system from diskette.
load:
	call	setreg		; set up ah, cx, dx
	mov	bx,[disksec]	; bx = number of next sector to read
	add	bx,2		; diskette sector 1 goes at 1536 ("sector" 3)
	shl	bx,1		; multiply sector number by 32
	shl	bx,1		; ditto
	shl	bx,1		; ditto
	shl	bx,1		; ditto
	shl	bx,1		; ditto
	mov	es,bx		; core address is es:bx (with bx = 0)
	xor	bx,bx		; see above
	add	[disksec],ax	; ax tells how many sectors to read
	mov	ah,2		; opcode for read
	int	13h		; call the BIOS for a read
	jb	error		; jump on diskette error
	mov	ax,[disksec]	; see if we are done loading
	cmp	ax,[final]	; ditto
	jb	load		; jump if there is more to load

; Loading done.  Finish up.
        mov     dx,03F2h        ; kill the motor
        mov     ax,000Ch
        out     dx,al

        cli
        mov     bx,[tracksiz]	; fsck expects # sectors/track in bx
        mov     ax,[fsck_ds]    ; set segment registers
        mov     ds,ax           ; when sep I&D ds != cs
        mov     es,ax           ; otherwise they are the same.
        mov     ss,ax           ; This gets patched by 'build'

        ;db      0x2E
        ;db      0xFF
        ;db      0x2E
        ;dw      fsck_pc
        jmp     WORD far[cs:fsck_pc]
        ;jmp     DWORD PTR cs:fsck_pc ; call the booted program
                                ; its address is at 508 (dec)

; Given the number of the next disk block to read, disksec, compute the
; cylinder, sector, head, and number of sectors to read as follows:
; ah = # sectors to read;  cl = sector #;  ch = cyl;  dh = head; dl = 0
setreg:	
	mov	si,[tracksiz]	; 9 (PC) or 15 (AT) sectors per track
	mov 	ax,[disksec]	; ax = next sector to read
	xor	dx,dx		; dx:ax = 32-bit divident
	div	si		; divide sector # by track size
	mov	cx,ax		; cx = track #; dx = sector (0-origin)
	mov	bx,dx		; bx = sector number (0-origin)
	mov	ax,[disksec]	; ax = next sector to read
	add	ax,si		; ax = last sector to read + 1
	dec	ax		; ax = last sector to read
	xor	dx,dx		; dx:ax = 32-bit dividend
	div	WORD[tracksiz]	; divide last sector by track size
	cmp	al,cl		; is starting cyl = ending cyl
	je	set1		; jump if whole read on 1 cylinder
	sub	si,dx		; compute lower sector count
	dec	si		; si = # sectors to read

; Check to see if this read crosses a 64K boundary (128 sectors).
; Such calls must be avoided.  The BIOS gets them wrong.
set1:	mov	ax,[disksec]	; ax = next sector to read
	add	ax,2		; disk sector 1 goes in core sector 3
	mov	dx,ax		; dx = next sector to read
	add	dx,si		; dx = one sector beyond end of read
	dec	dx		; dx = last sector to read
	shl	ax,1		; ah = which 64K bank does read start at
	shl	dx,1		; dh = which 64K bank foes read end in
	cmp	ah,dh		; ah != dh means read crosses 64K boundary
	je	set2		; jump if no boundary crossed
	shr	dl,1		; dl = excess beyond 64K boundary
	xor	dh,dh		; dx = excess beyond 64K boundary
	sub	si,dx		; adjust si
	dec	si		; si = number of sectors to read

set2:	mov	ax,si		; ax = number of sectors to read
	xor	dx,dx		; dh = head, dl = drive
	mov	dh,cl		; dh = track
	and	dh,1		; dh = head
	mov	ch,cl		; ch = track to read
	shr	ch,1		; ch = cylinder
	mov	cl,bl		; cl = sector number (0-origin)
	inc	cl		; cl = sector number (1-origin)
	xor	dl,dl		; dl = drive number (0)
	ret			; return values in ax, cx, dx


;-------------------------------+
;    error & print routines     ;
;-------------------------------+
;
error:
        push    ax
        mov     bx, fderr
        call    print           ; print msg
	xor	cx,cx
   err1:mul	bl		; delay
	loop	err1
	int	19h

prhexbyte:
	mov	ah,al
	shr	al,1
	shr	al,1
	shr	al,1
	shr	al,1
	call	printhex
	mov	al,ah
	and	al,0Fh
	call	printhex
	ret

printhex:                          ; print string (bx)
	push    ax
	mov	ah,00Fh
	cmp	al,010d
	jae	d1
	add	al,'0'
	jmp	prh1
d1:	add	al,'A'-010d

prh1:   mov	[es:di],ax
        inc	di
	inc	di
	pop	ax
        ret

print:                          ; print string (bx), ax destroyed
        mov     al,[bx]         ; al contains char to be printed
        test    al,al           ; null char?
        jne     prt1            ; no
        ret                     ; else return
prt1:   mov     ah,14           ; 14 = print char
        inc     bx              ; increment string pointer
        push    bx              ; save bx
        mov     bl,1            ; foreground color
	xor	bh,bh		; page 0
        int     10h             ; call BIOS VIDEO_IO
        pop     bx              ; restore bx
        jmp     print           ; next character


disksec:  DW 	1
tracksiz: DW 	18		; changed to 9 for 360K diskettes
pcpar:	  DB	0DFh, 2, 25, 2, 9, 02Ah, 0FFh, 050h, 0F6h, 1, 3  ; for PC
atpar:	  DB	0DFh, 2, 25, 2,15, 01Bh, 0FFh, 054h, 0F6h, 1, 8  ; for AT

fderr:    DB    "Read error.  Please reboot ",0Dh,0Ah,0
greet:    DB    0Dh,"Booting MINIX 1.0 ",0Dh,0Ah,0

        times 502-($-$$) db 0
;        ORG     504     ; NOTE: there will be no error reported if
                         ; your code crosses this org-directive !
final:    DW     0       ; # sectors to read (patched in by build)
fsck_ds:  DW     0       ; ds-value for sep I&D
fsck_pc:  DW     0       ; build patches these locations to the
fsck_cs:  DW     0       ; starting address of the booted program
          DW     0xaa55

;CODE    ENDS

;        END
