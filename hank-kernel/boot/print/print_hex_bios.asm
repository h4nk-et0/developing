print_hex_bios:
	mov dx,bx
        mov ah,0x0e
        mov al,'0'
        int 0x10
        mov al,'x'
        int 0x10
        mov cl,12
        loop_p_hex_bios:
                mov bx,dx
                shr bx,cl
                and bl,0x0f
                cmp bl,0x09
                jg p_hex_bios_letter
                add bx,0x30
                jmp p_hex_bios
                p_hex_bios_letter:
                        add bx,0x57
                p_hex_bios:
                        mov ah,0x0e
                        mov al,bl
                        int 0x10
                cmp cl,0
                je end_loop_p_hex_bios
                sub cl,4
                jmp loop_p_hex_bios
        end_loop_p_hex_bios:
                ret
