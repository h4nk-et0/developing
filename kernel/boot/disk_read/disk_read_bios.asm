disk_read_bios:
	mov ax,0
	mov es,ax
	mov ah,0x02
	mov dl,[disk_drive]
	mov ch,0
	mov dh,0
	mov cl,0x02
	mov al,15
	int 0x13

	jc disk_read_error

	cmp al,15
	jne disk_read_error

	ret
