ll $t0, $zero, $zero, 0 # t0=MEM[0]
add $t1, $t1,$imm, 1 #t1=t1+1
add $t0, $t1, $t0, 0 #t0=t0+$t1
sc  $t0, $zero, $zero, 0 #MEM[0]=t0
beq $zero, $t0, $zero, 0 #if t0==0 (sc failed), loop again from the start
sub $t1, $t1, $imm, 1 #t1=t1-1 (delay slot)
bne $zero, $t1, $imm, 127 #if (t1 !=127) loop again from the start
add $t2, $imm, $zero, 256 #t2=256
lw $t1, $t2, $zero, 0 # t1=MEM[256] #make conflict to save MEM[0] in the main memory
halt $zero, $zero, $zero, 0
halt $zero, $zero, $zero, 0
halt $zero, $zero, $zero, 0
halt $zero, $zero, $zero, 0
#t0=2
#t1=3