[org 0x7c00]

mov [disk_drive],dl

call print_nl_bios
mov bx,start_boot_msg
call print_string_bios
call print_nl_bios

mov bx,mode_16_bit_msg
call print_string_bios
call print_nl_bios

mov bp,0x9000
mov sp,bp

mov bx,load_kernel_msg
call print_string_bios

mov bx,[kernel_offset]
mov dh,[disk_drive]
call disk_read_bios

mov bx,proc_cond_ok
call print_string_bios
call print_nl_bios

call switch_prot_mode	; it never returns

jmp end_bootstrap

disk_read_error:
	mov bx,proc_cond_fail
	call print_string_bios
	call print_nl_bios

	mov bx,disk_error_msg
	call print_string_bios
	call print_nl_bios

end_bootstrap:
	jmp end_bootstrap


;replace this path to the valid path of "boot" folder

%include "/media/darkden/my_code/projects/project_hank/hank-kernel/boot/print/print_hex_bios.asm"
%include "/media/darkden/my_code/projects/project_hank/hank-kernel/boot/print/print_nl_bios.asm"
%include "/media/darkden/my_code/projects/project_hank/hank-kernel/boot/print/print_string_bios.asm"
%include "/media/darkden/my_code/projects/project_hank/hank-kernel/boot/disk_read/disk_read_bios.asm"
%include "/media/darkden/my_code/projects/project_hank/hank-kernel/boot/gdt/gdt.asm"
%include "/media/darkden/my_code/projects/project_hank/hank-kernel/boot/switch_prot_mode/switch_prot_mode.asm"
%include "/media/darkden/my_code/projects/project_hank/hank-kernel/boot/print/print_string_32_bit.asm"

[bits 32]

begin_prot_mode:

	mov ch,13
	mov cl,33
	mov ebx,proc_cond_ok
	call print_string_32_bit

	call [kernel_offset]

	jmp $

start_boot_msg:
	db '[*] booting hank-OS ...',0
mode_16_bit_msg:
	db '[*] running in 16 bit mode ...',0
load_kernel_msg:
	db '[*] loading kernel ...',0
disk_error_msg:
	db '[-] disk read error ...',0
switch_32_bit_msg:
	db '[*] switching to 32 bit mode ...',0
proc_cond_ok:
	db '[ok]',0
proc_cond_fail:
	db '[fail]',0
disk_drive:
	db 0
kernel_offset:
	dw 0x1000

times 510-($-$$) db 0
dw 0xaa55
