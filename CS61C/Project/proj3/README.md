# CS61CPU

Look ma, I made a CPU! Here's what I did:

## in part A
* for task1 ALU: use multiplexer to distinguish different switch value,as
most computation components are already implemented in logisim. Notice `mul`, `mulhu` and `mulh`
* for task2 RegFile:
  * use two MUXes to determine output `rs1` and `rs2`
  * use one DeMUXes to determine which register is to be writen by setting correspond reg enable signal 
  * implement 32 reg from x0 to x31 by using provided reg components in logisim
  * notice x0 is always 0
* for task3 addi:
  * for single:
    * implement Immediate Generator and set signals constant in Control Logic for addi operation
    * implement Processor, and use splitter to decode instruction, see slides for reference
  * for pipelined:
    * add reg between old instruction and new instruction, old PC and new PC.
    * notice the EXECUTE stage uses new PC and new instruction including Control Logic and address counting.

## in part B
* for task 4
  * the most difficult part is control logic design. I choose hard-ired control which is basically combinational logic
    * done it by dealing with one instruction format each time, add gates design from less to more, just like what course slides do 
  * the processor design is relatively simple, just follow course slides instruction
    * for pipelined design
      * the IF and EX stages have **different** PC values. In a pipelined processor, each stage operates on a different instruction simultaneously. 
      The IF stage fetches an instruction using the current PC value, while the EX stage executes an instruction that was fetched in a previous cycle with a different PC value.
      * **need to store** the PC between the pipelining stages.
      The PC value associated with an instruction must be passed from the IF stage to the EX stage through a pipeline register. This is necessary because:
      the EX stage needs the PC for calculating branch targets
      * To MUX a nop into the instruction stream, place it **before** the instruction register. 
      The MUX should be placed before the instruction register that holds the instruction for the EX stage. 
      This allows the pipeline control logic to replace a fetched instruction with a nop (bubble) when needed (e.g., for hazard handling).
      * While the EX stage executes a nop, the IF stage should request the target address of the branch/jump for the next instruction fetch. 
      This is different from the normal case where IF would sequentially fetch the next instruction at PC + 4.
    * illustrate control hazards handle with a cycle-by-cycle example:
      * Assume:
        * Cycle 0:
          * IF: fetching instruction at address X (which is a taken branch)
          * EX: previous instruction (whatever it was)
        * Cycle 1:
          * EX: the branch instruction (at X) is now in EX. It calculates the target address T and decides taken.
          * IF: fetching instruction at X+4 (the next sequential instruction)
          At the end of cycle 1, we know the branch is taken, so we:
            - Set the PC to T for the next cycle.
            - Kill the instruction that was fetched (at X+4) by turning it into a nop.
        * Cycle 2:
          * IF: fetching instruction at T
          * EX: the nop (the killed instruction from X+4)
* for task 5
  * consider adequate testing for every possible instruction.
  * I write a script `create_test.sh` in `./tests/part_b/custom` which can automatically create test files for all .s files I create in `input` directory 