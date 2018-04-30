
%include "../h/model.h"

KERNRL_START equ 0x60
STACK_SIZE   equ 4096

extern _cmain
global _bios_putc,_bios_getc

        
segment _TEXT
cpu 8086
bits 16
..start:
        mov ax,DGROUP
        xor ax,ax
        mov bx,_edata
        mov cx,_end
        sub cx,bx
        sar cx,1
   lp1: 
        mov [bx],ax
        add bx,2
        loop lp1
        
        mov sp,_fsck_stack+STACK_SIZE
        call _cmain
        mov bx,ax
        
        ;jmp $
        cli
        mov dx,KERNRL_START
        mov ds,dx
        mov es,dx
        mov ss,dx
        jmp KERNRL_START:0x0

;void bios_putc(int c)        
_bios_putc:
	push bp
	mov bp, sp
	push di
	push si
	push ax
	push dx
	
	mov al, [bp+4]
	mov bh, 0
	mov ah, 0x0e
	int 0x10
	
	pop dx
	pop ax
	pop si
	pop di
	mov sp, bp
	pop bp
	ret

;int bios_getc(void)        
_bios_getc:
        xor     ah,ah
        int     16h
        ret
        
segment _DATABEG
        
segment _DATA

segment _DATAEND
_edata:

segment _BSS
_fsck_stack: resb STACK_SIZE

segment _BSSEND
_end:

segment STACK
