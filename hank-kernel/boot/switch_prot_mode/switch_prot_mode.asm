switch_prot_mode:
	cli
	lgdt [gdt_descriptor]

	mov bx,switch_32_bit_msg
	call print_string_bios

	mov eax,cr0
	or eax,0x01
	mov cr0,eax

	jmp code_seg:init_prot_mode

[bits 32]

init_prot_mode:
	mov ax,data_seg
	mov ds,ax
	mov ss,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	mov ebp,0x90000
	mov esp,ebp

	call begin_prot_mode
