.data
    startx: .word 5
    starty: .word 10
.code

mov r0, [startx]   ; m=5
mov r1, [starty]  ; n=10
mov r2, 0   ; accumulator

outer_loop:
    cmp r0, 0
    jle done
    mov r3, r1

inner_loop:
    cmp r3, 0
    jle outer_next
    mul r4, r0, r3    ; r4 = r0*r3
    add r2, r2, r4
    sub r3, r3, 1
    jmp inner_loop

outer_next:
    sub r0, r0, 1
    jmp outer_loop

done:
ret

main:
    call outer_loop
    ret
