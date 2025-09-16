.globl dot

.text
# =======================================================
# FUNCTION: Dot product of 2 int vectors
# Arguments:
#   a0 (int*) is the pointer to the start of v0
#   a1 (int*) is the pointer to the start of v1
#   a2 (int)  is the length of the vectors
#   a3 (int)  is the stride of v0
#   a4 (int)  is the stride of v1
# Returns:
#   a0 (int)  is the dot product of v0 and v1
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 75.
# - If the stride of either vector is less than 1,
#   this function terminates the program with error code 76.
# =======================================================
dot:
    ble a2, zero, error_length_exit
    ble a3, zero, error_stride_exit
    ble a4, zero, error_stride_exit

    li t1, 0
    li t6, 0
    j loop_start

error_length_exit:
    li a1, 75
    jal exit2

error_stride_exit:
    li a1, 76
    jal exit2

loop_start:
    bge t1, a2, loop_end

loop_continue:
    mul t2, t1, a3
    slli t2, t2, 2
    add t2, a0, t2

    mul t3, t1, a4
    slli t3, t3, 2
    add t3, a1, t3

    lw t4, 0(t2)
    lw t5, 0(t3)

    mul t4, t4, t5
    add t6, t4, t6

    addi t1, t1, 1

    j loop_start

loop_end:

    mv a0, t6
    ret
