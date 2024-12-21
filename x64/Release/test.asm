
; Calculate factorial(5) and store the result in R1
; R0 = 5  (the number we want the factorial of)
; R1 = 1  (accumulator for result)
; Loop until R0 == 1

mov r0, 7
mov r1, 1

start:
    cmp r0, 1
    jle end     ; if R0 <= 1, jump to end
    mul r1, r0  ; R1 = R1 * R0
    sub r0, 1   ; R0 = R0 - 1
    jmp start

end:
    ; At this point, R1 contains factorial(5)=120
    ; End the program
    ret