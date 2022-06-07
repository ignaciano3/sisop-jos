/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(trap_0, T_DIVIDE); // 0
TRAPHANDLER_NOEC(trap_1, T_DEBUG); // 1
TRAPHANDLER_NOEC(trap_2, T_NMI); // 2
TRAPHANDLER_NOEC(trap_3, T_BRKPT); // 3
TRAPHANDLER_NOEC(trap_4, T_OFLOW); // 4
TRAPHANDLER_NOEC(trap_5, T_BOUND); // 5
TRAPHANDLER_NOEC(trap_6, T_ILLOP); // 6
TRAPHANDLER_NOEC(trap_7, T_DEVICE); // 7

TRAPHANDLER(trap_8, T_DBLFLT); // 8
// Trap 9 is reserved.
TRAPHANDLER(trap_10, T_TSS); // 10
TRAPHANDLER(trap_11, T_SEGNP); // 11
TRAPHANDLER(trap_12, T_STACK); // 12
TRAPHANDLER(trap_13, T_GPFLT); // 13
TRAPHANDLER(trap_14, T_PGFLT); // 14
// Trap 15 is reserved.
TRAPHANDLER_NOEC(trap_16, T_FPERR); // 16
TRAPHANDLER_NOEC(trap_17, T_ALIGN); // 17
TRAPHANDLER_NOEC(trap_18, T_MCHK); // 18
TRAPHANDLER_NOEC(trap_19, T_SIMDERR); // 19
TRAPHANDLER_NOEC(trap_48, T_SYSCALL); // 48

TRAPHANDLER_NOEC(trap_32, IRQ_TIMER + IRQ_OFFSET); //32



/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
    // Push registers
    pushl %ds;
    pushl %es;
    pushal;
    // Load GD_KD
    mov $(GD_KD), %ax;
    mov %ax, %ds;
    mov %ax, %es;
    // Push Trapframe pointer as param to trap
    pushl %esp;
    call trap;







