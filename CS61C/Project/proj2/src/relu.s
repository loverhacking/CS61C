.globl relu

.text
# ==============================================================================
# FUNCTION: Performs an inplace element-wise ReLU on an array of ints
# Arguments:
# 	a0 (int*) is the pointer to the array
#	a1 (int)  is the # of elements in the array
# Returns:
#	None
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 78.
# ==============================================================================
relu:
    # Check if the number of elements is less than 1
    ble a1, zero, error_exit  # If a1 <= 0, jump to error

    # Initialize loop counter (t1 = 0)
    mv t1, zero
    j loop_start

error_exit:
    li a1, 78
    jal exit2

loop_start:
    bge t1, a1, loop_end

loop_continue:
    slli t2, t1, 2
    add t2, a0, t2
    lw t3, 0(t2)
    addi, t1, t1, 1
    bge t3, zero, loop_start
    sw zero, 0(t2)
    j loop_start

loop_end:
	ret
