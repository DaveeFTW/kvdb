@ exceptions.S -- exception handler for debugger
@
@ Copyright (C) 2018 molecule
@
@ This software may be modified and distributed under the terms
@ of the MIT license.  See the LICENSE file for details.
@ 
  .global g_abt_excp_count
  .global insn_fault_triggered
  .global mem_fault_triggered
  .global insn_undef_triggered
  .global debug_asm

  .arch armv7-a
  .text
  .align 4
  .syntax unified

.macro DUMP_REG reg
  cpsie i
  stmfd sp!, {r0-r3,r12,lr}
  mov r0, \reg
  blx debug_asm
  ldmfd sp!, {r0-r3,r12,lr}
  cpsid i
.endm

  .arm
  .global debugger_pabt_handler
debugger_pabt_handler:
  .word 0x0
  .word 0x0
  mov   r4, r0
  mov   r3, r1
  ldr   r2, [r0, #0x44] @ spsr
  ldr   r1, [r0, #0x3c] @ lr
  stmfd sp!, {lr}
  sub   sp, sp, #4
  cpsie i
  blx   insn_fault_triggered
  cpsid i
  mov   r1, r0
  mov   r0, r4
  add   sp, sp, #4
  ldmfd sp!, {lr}
  and   r2, r1, #0xf
  cmp   r2, #0
  bne   Lnext_pabt_handler
  mrc   p15, 0, r1, c0, c0, 5 @ [<] MPIDR (Multiprocessor Affinity Register)
  and   r1, r1, #0xf
  ldr   r2, =g_abt_excp_count
  ldr   r2, [r2]
  cmp   r2, #0
  beq   Ldie1
  add   r2, r2, r1, lsl#2
  ldr   r3, [r2]
  subs  r3, #1
  bne   Ldie1
  str   r3, [r2]
  mov   r1, #0
  mcr   p15, 0, r1, c5, c0, 0 @ [>] DFSR (Data Fault Status Register)
  mcr   p15, 0, r1, c5, c0, 1 @ [>] IFSR (Instruction Fault Status Register)
  mcr   p15, 0, r1, c5, c1, 0 @ [>] ADFSR (Auxiliary Data Fault Status Register)
  mcr   p15, 0, r1, c5, c1, 1 @ [>] AIFSR (Auxiliary Instruction Fault Status Register)
  mcr   p15, 0, r1, c6, c0, 0 @ [>] DFAR (Data Fault Address Register)
  mcr   p15, 0, r1, c6, c0, 2 @ [>] IFAR (Instruction Fault Address Register)
  ldr   r1, [r0, #0x60] @ tpidrprw
  mcr   p15, 0, r1, c13, c0, 4 @ TPIDRPRW
  b     restore_thread
Lnext_pabt_handler:
  ldr   r2, =debugger_pabt_handler
  ldr   r2, [r2]
  add   r2, r2, #8
  mov   pc, r2
Ldie1:
  b     Ldie1

  .arm
  .global debugger_dabt_handler
debugger_dabt_handler:
  .word 0x0
  .word 0x0
  mov   r4, r0
  mov   r3, r1
  ldr   r2, [r0, #0x44] @ spsr
  ldr   r1, [r0, #0x3c] @ lr
  stmfd sp!, {lr}
  sub   sp, sp, #4
  cpsie i
  blx   mem_fault_triggered
  cpsid i
  mov   r1, r0
  mov   r0, r4
  add   sp, sp, #4
  ldmfd sp!, {lr}
  and   r2, r1, #0xf
  cmp   r2, #0
  bne   Lnext_dabt_handler
  mrc   p15, 0, r1, c0, c0, 5 @ [<] MPIDR (Multiprocessor Affinity Register)
  and   r1, r1, #0xf
  ldr   r2, =g_abt_excp_count
  ldr   r2, [r2]
  cmp   r2, #0
  beq   Ldie2
  add   r2, r2, r1, lsl#2
  ldr   r3, [r2]
  subs  r3, #1
  bne   Ldie2
  str   r3, [r2]
  mov   r1, #0
  mcr   p15, 0, r1, c5, c0, 0 @ [>] DFSR (Data Fault Status Register)
  mcr   p15, 0, r1, c5, c0, 1 @ [>] IFSR (Instruction Fault Status Register)
  mcr   p15, 0, r1, c5, c1, 0 @ [>] ADFSR (Auxiliary Data Fault Status Register)
  mcr   p15, 0, r1, c5, c1, 1 @ [>] AIFSR (Auxiliary Instruction Fault Status Register)
  mcr   p15, 0, r1, c6, c0, 0 @ [>] DFAR (Data Fault Address Register)
  mcr   p15, 0, r1, c6, c0, 2 @ [>] IFAR (Instruction Fault Address Register)
  ldr   r1, [r0, #0x60] @ tpidrprw
  mcr   p15, 0, r1, c13, c0, 4 @ TPIDRPRW
  b     restore_thread
Lnext_dabt_handler:
  ldr   r2, =debugger_dabt_handler
  ldr   r2, [r2]
  add   r2, r2, #8
  mov   pc, r2
Ldie2:
  b     Ldie2

  .arm
  .global debugger_und_handler
debugger_und_handler:
  .word 0x0
  .word 0x0
  mov   r4, r0
  mov   r3, r1
  ldr   r2, [r0, #0x44] @ spsr
  ldr   r1, [r0, #0x3c] @ lr
  stmfd sp!, {lr}
  sub   sp, sp, #4
  cpsie i
  blx   insn_undef_triggered
  cpsid i
  mov   r1, r0
  mov   r0, r4
  add   sp, sp, #4
  ldmfd sp!, {lr}
  and   r2, r1, #0xf
  cmp   r2, #0
  bne   Lnext_und_handler
  mrc   p15, 0, r1, c0, c0, 5 @ [<] MPIDR (Multiprocessor Affinity Register)
  and   r1, r1, #0xf
  ldr   r2, =g_abt_excp_count
  ldr   r2, [r2]
  cmp   r2, #0
  beq   Ldie3
  add   r2, r2, r1, lsl#2
  ldr   r3, [r2]
  subs  r3, #1
  bne   Ldie3
  str   r3, [r2]
  mov   r1, #0
  mcr   p15, 0, r1, c5, c0, 0 @ [>] DFSR (Data Fault Status Register)
  mcr   p15, 0, r1, c5, c0, 1 @ [>] IFSR (Instruction Fault Status Register)
  mcr   p15, 0, r1, c5, c1, 0 @ [>] ADFSR (Auxiliary Data Fault Status Register)
  mcr   p15, 0, r1, c5, c1, 1 @ [>] AIFSR (Auxiliary Instruction Fault Status Register)
  mcr   p15, 0, r1, c6, c0, 0 @ [>] DFAR (Data Fault Address Register)
  mcr   p15, 0, r1, c6, c0, 2 @ [>] IFAR (Instruction Fault Address Register)
  ldr   r1, [r0, #0x60] @ tpidrprw
  mcr   p15, 0, r1, c13, c0, 4 @ TPIDRPRW
  b     restore_thread
Lnext_und_handler:
  ldr   r2, =debugger_und_handler
  ldr   r2, [r2]
  add   r2, r2, #8
  mov   pc, r2
Ldie3:
  b     Ldie3

  .arm
restore_thread:
  ldr   r1, [r0, #0x5c] @ TPIDRURO
  mcr   p15, 0, r1, c13, c0, 3 @ [>] TPIDRURO (PL0 Read-Only Software Thread ID Register)
  ldr   r1, [r0, #0x64] @ TTBR1
  ldr   r2, [r0, #0x54] @ CONTEXTIDR
  mov   r3, #0
  mcr   p15, 0, r3, c13, c0, 1 @ [>] CONTEXTIDR (Context ID Register)
  isb   sy
  mcr   p15, 0, r1, c2, c0, 1 @ [>] TTBR1 (Translation Table Base Register 1)
  isb   sy
  mcr   p15, 0, r2, c13, c0, 1 @ [>] CONTEXTIDR (Context ID Register)
  ldr   r1, [r0, #0x6c] @ DACR
  mcr   p15, 0, r1, c3, c0, 0 @ [>] DACR (Domain Access Control Register)
  ldr   r1, [r0, #0x80] @ PAR
  mcr   p15, 0, r1, c7, c4, 0 @ [>] PAR (Physical Address Register)
  ldr   r1, [r0, #0x40] @ save_fp_regs
  cmp   r1, #0
  beq   Lno_fp
  ldr   r2, [r0, #0x50] @ fpexc
  ldr   r3, [r0, #0x4c] @ fpscr
  vmsr  fpexc, r2
  vmsr  fpscr, r3
  adds  r2, r0, #0x100 @ fp_regs
  vldmia r2!, {d0-d15}
  vldmia r2!, {d16-d31}
Lno_fp:
  ldr   r1, [r0, #0x48] @ cpacr
  mcr   p15, 0, r1, c1, c0, 2 @ [>] CPACR (Architectural Feature Access Control Register)
  ldr   r1, [r0, #0x3C] @ lr_abt
  cps   #0x17   @ abort mode
  @mcr   p15, 0, r4, c12, c0, 0 @ [<] VBAR (Vector Base Address Register)
  @ldr   r4, [r4, #0x24]
  @cmp   r4, #0
  @mov   r5, #1
  @strne r5, [r4, #0x100]
  movs  r12, r0
  ldmia r12!, {r0-r11}
  adds  r12, r12, #4
  ldmia r12, {sp,lr}^
  subs  r12, r12, #0x34
  ldr   lr, [r12, #0x44] @ spsr
  str   lr, [sp, #-4]!
  ldr   lr, [r12, #0x3C] @ lr_abt
  str   lr, [sp, #-4]!
  ldr   r12, [r12, #0x30]
  clrex
  rfefd sp!
