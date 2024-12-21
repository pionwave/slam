
; R0 = a, R1 = b
; We'll compute gcd(a,b) and store in R0

    mov r0, 11672
    mov r1, 27

gcd_start:
    cmp r1, 0
    je gcd_done
    ; divide r0 by r1
    ; We'll emulate remainder using DIV:
    ; After DIV r0, r1: R0 = R0/R1, maybe store remainder in another way if needed.
    ; If your instruction set doesn't handle remainder, you can do: remainder = original_a - (quotient * b)
    ; For simplicity, let's assume remainder can be found:
    push r0
    push r1

    ; remainder = a - (a/b)*b
    div r0, r0, r1     ; now R0 = a/b
    ; Need original a and b
    pop r1          ; b
    pop r3          ; a
    mul r0, r0, r1      ; (a/b)*b
    sub r3, r3, r0      ; remainder = a - (a/b)*b
    mov r0, r1
    mov r1, r3
    jmp gcd_start

gcd_done:
ret