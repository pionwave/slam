; Assume memory is zeroed. We'll store values at addresses 200,204,208 and sum them.
; Have to be careful with memory, as the program & heap exist in the same space
; don't corrupt your program~!!

.data

	heaptr: .word 1024

.code

memtest:
	mov r0, 10
	mov r5, 1000
	store [r5], r0   ; mem[100..103] = 10
	mov r0, 20
	store [1028], r0   ; mem[104..107] = 20
	mov r0, 30
	store [1032], r0   ; mem[108..111] = 30
	mov [heaptr], 300
	mov r6, [heaptr]

	mov r1, 0
	load r2, [r5]  ; r2=10
	add r1, r1, r2
	load r2, [1028]  ; r2=20
	add r1, r1, r2
	load r2, [1032]  ; r2=30
	add r1, r1, r2  ; r1=10+20+30=60

	ret

main:
	call memtest
	ret
