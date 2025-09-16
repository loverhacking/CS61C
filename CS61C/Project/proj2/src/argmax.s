.globl argmax

.text
# =================================================================
# FUNCTION: Given a int vector, return the index of the largest
#	element. If there are multiple, return the one
#	with the smallest index.
# Arguments:
# 	a0 (int*) is the pointer to the start of the vector
#	a1 (int)  is the # of elements in the vector
# Returns:
#	a0 (int)  is the first index of the largest element
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 77.
# =================================================================
argmax:
    # Check if the number of elements is less than 1
    ble a1, zero, error_exit  # If a1 <= 0, jump to error

    # Initialize loop counter (t1 = 0)
    mv t1, zero
    mv t5, zero
    lw t2, 0(a0)

    j loop_start

error_exit:
    li a1, 77
    jal exit2

loop_start:
    addi t1, t1, 1
    bge t1, a1, loop_end

loop_continue:
    slli t3, t1, 2
    add t3, a0, t3
    lw t4, 0(t3)
    blt t4, t2, loop_start
    beq t4, t2, loop_start
    mv t2, t4
    mv t5, t1
    j loop_start


loop_end:
    mv a0, t5
    ret
