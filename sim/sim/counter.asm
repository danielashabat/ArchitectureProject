ll $R2, $zero, $zero, 0 # R2=MEM[0]
add $R3, $R3,$imm, 1 #R3=R3+1
add $R2, $R2, $imm, 1 #R2=R2+1
sc  $R2, $zero, $zero, 0 #MEM[0]=R2,if succed, R2=1,o.w R2=0
beq $zero, $R2, $zero, 0 #if R2==0 (sc failed), loop again from the start
sub $R3, $R3, $imm, 1 #R3=R3-1 (delay slot)
bne $zero, $R3, $imm, 127 #if (R3 !=127) loop again from the start
add $R3, $R3,$imm, 1 #R3=R3+1
add $R4, $imm, $zero, 256 #R4=256
lw $R3, $R4, $zero, 0 # R3=MEM[256] #make conflict to save MEM[0] in the main memory
halt $zero, $zero, $zero, 0
halt $zero, $zero, $zero, 0
halt $zero, $zero, $zero, 0
halt $zero, $zero, $zero, 0
