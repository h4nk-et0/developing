print_string_bios:
	mov ah,0x0e
	loop_p_string_bios:
		mov al,[bx]
		cmp al,0
		je end_p_string_bios
		int 0x10
		inc bx
		jmp loop_p_string_bios
	end_p_string_bios:
		ret
