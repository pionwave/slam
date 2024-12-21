
.data
    valist: .word 99,38,11

.code

mov r0, 10    ; n = 10
mov r1, 0     ; f0 = 0
mov r2, 1     ; f1 = 1

fib_start:
    cmp r0, 0
    jle fib_end   ; if r0 <= 0 end
    add r3, r1, r2  ; r3 = r1 + r2
    mov r1, r2
    mov r2, r3
    sub r0, r0, 1
    jmp fib_start

fib_end:
ret