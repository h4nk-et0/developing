[bits 32]

video_memory equ 0xb8000
white_on_black equ 0x0f

print_string_32_bit:
	dec ch
	dec cl
	mov edx,0
	mov eax,0
	mov dl,ch
	mov ax,80
	mul dx
	mov ch,0
	add ax,cx
	mov dx,2
	mul dx
	mov edx,eax
	add edx,video_memory

	p_str_32_bit_loop:
		mov al,[ebx]
		mov ah,white_on_black
		cmp al,0
		je p_str_32_bit_end
		mov [edx],ax
		inc ebx
		add edx,2
		jmp p_str_32_bit_loop

	p_str_32_bit_end:
		ret
