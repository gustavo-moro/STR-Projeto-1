/****************************************************************************
* MInimal Real-time Operating System (MiROS), GNU-ARM port.
* version 1.26 (matching lesson 26, see https://youtu.be/kLxxXNCrY60)
*
* This software is a teaching aid to illustrate the concepts underlying
* a Real-Time Operating System (RTOS). The main goal of the software is
* simplicity and clear presentation of the concepts, but without dealing
* with various corner cases, portability, or error handling. For these
* reasons, the software is generally NOT intended or recommended for use
* in commercial applications.
*
* Copyright (C) 2018 Miro Samek. All Rights Reserved.
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* Git repo:
* https://github.com/QuantumLeaps/MiROS
****************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include "miros.h"
#include "qassert.h"
#include "stm32f1xx.h"
#include <assert.h>
#include <time.h>

Q_DEFINE_THIS_FILE

OSThread * volatile OS_curr; /* pointer to the current thread */
OSThread * volatile OS_next; /* pointer to the next thread to run */
uint8_t MAX_THREADS = 7;
OSThread *OS_thread[7]; /* array of threads started so far */

uint32_t OS_readySet = 0; /* bitmask of threads that are ready to run */
uint32_t OS_delayedSet = 0; /* bitmask of threads that are delayed */
uint32_t OS_aperiodicSet = 0;

int priorities[7];

#define LOG2(x) (32U - __builtin_clz(x))

OSThread idleThread;
void main_idleThread() {
    while (1) {
        OS_onIdle();
    }
}

void OS_init(void *stkSto, uint32_t stkSize) {
    /* set the PendSV interrupt priority to the lowest level 0xFF */
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);

    /* start idleThread thread */
    OSThread_start(&idleThread,
                   0U, /* idle thread priority */
                   &main_idleThread,
                   stkSto, stkSize, 1);
}

void fix_priorities(){
	priorities[0] = 33;
	for(int i = 1; i < MAX_THREADS; i++){
		OSThread *current_thread = OS_thread[i];
		if(current_thread->tipo == 1){
			uint32_t prio = 1;
			for(int j = 1; j < MAX_THREADS; j++){
				OSThread *other_thread = OS_thread[j];
				if(other_thread->tipo == 1 && current_thread->period > other_thread->period){
					prio++;
				}
			}
			priorities[i] = prio;
		} else{
			priorities[i] = 33;
		}
	}
}

OSThread* find_next_thread(){
    OSThread *next_thread = NULL;
    uint32_t highest_priority = UINT32_MAX;
    uint32_t bit;
    for (int i = 1; i <= MAX_THREADS/2; i++) { // MAX_THREADS é o número total de threads
        bit = (1U << (i - 1U));
    	if (OS_readySet & bit) { // Verifica se a thread i está pronta
            OSThread *current_thread = OS_thread[i];
            if (priorities[i] < highest_priority) {
                highest_priority = priorities[i];
                next_thread = current_thread;
            }
        }
    }
    return next_thread;
}

OSThread* sched_aperiodic(){
    OSThread *next_thread = NULL;
    uint32_t bit;
    for (int i = 4; i < MAX_THREADS; i++) { // MAX_THREADS é o número total de threads
        bit = (1U << (i - 1U));
        if(bit & OS_readySet){
            OSThread *current_thread = OS_thread[i];
            next_thread = current_thread;
        }
    }
    return next_thread;
}

void OS_sched(void) {
    /* choose the next thread to execute... */
	int periodics = 0;
	uint32_t bit;
	for(int i = 1; i <= MAX_THREADS/2; i++){
		bit = (1U << (i - 1U));
		if(bit & OS_readySet){
			periodics++;
		}
	}
    OSThread *next;
     /* Background Scheduling*/
    if(periodics == 0){
    	next = sched_aperiodic();
    	if(next == NULL) {
    		next = OS_thread[0];
    	}
    } else{
        next = find_next_thread();
        Q_ASSERT(next != (OSThread *)0);
    }


    /* trigger PendSV, if needed */
    if (next != OS_curr) {
        OS_next = next;
        //*(uint32_t volatile *)0xE000ED04 = (1U << 28);
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        __asm volatile("dsb");
//        __asm volatile("isb");
    }
    /*
     * DSB - whenever a memory access needs to have completed before program execution progresses.
     * ISB - whenever instruction fetches need to explicitly take place after a certain point in the program,
     * for example after memory map updates or after writing code to be executed.
     * (In practice, this means "throw away any prefetched instructions at this point".)
     * */
}


void OS_run(void) {
    /* callback to configure and start interrupts */
    OS_onStartup();
    fix_priorities();
    __disable_irq();
    OS_sched();
    __enable_irq();

    /* the following code should never execute */
    Q_ERROR();
}

uint32_t current_time = 0;
void OS_tick(void) {
	current_time++;
    uint32_t workingSet = OS_delayedSet;
    while (workingSet != 0U) {
		OSThread *t = OS_thread[LOG2(workingSet)];
		uint32_t bit;
		Q_ASSERT((t != (OSThread *)0) && (t->timeout != 0U));

		bit = (1U << (t->id - 1U));
		--t->timeout;
		if (t->timeout == 0U) {
			OS_readySet   |= bit;  /* insert to set */
			OS_delayedSet &= ~bit; /* remove from set */
		}
			workingSet &= ~bit; /* remove from working set */
    }
}

void wait_next_period(){
	static int initialized = 0;
	if (!initialized) {
		srand(time(NULL));
		initialized = 1;
	}
	if(OS_curr->period == 0){
        uint32_t random_delay = 100 + rand() % 1000;
        OS_delay(random_delay);
	}else{
		OS_delay(OS_curr->period);
	}

}

void OS_delay(uint32_t ticks) {
    uint32_t bit;

    __asm volatile ("cpsid i");

    /* never call OS_delay from the idleThread */
    Q_REQUIRE(OS_curr != OS_thread[0]);

    OS_curr->timeout = ticks;

    bit = (1U << (OS_curr->id - 1U));
    OS_readySet   &= ~bit;
    OS_delayedSet |= bit;
    OS_sched();
    __asm volatile ("cpsie i");
}

/*DEFINIÇÃO DOS SEMÁFOROS*/

void sem_init(sem_t *sem, int init_count, int max_count){
	assert(init_count <= max_count);
	sem->cont = init_count;
	sem->max_cont =max_count;
}

void sem_wait(sem_t *sem){
	__disable_irq();
		while(sem->cont == 0){
			OS_delay(1U);
			__disable_irq();
		}
		sem->cont--;
		sem->prior = priorities[OS_curr->id];
		priorities[OS_curr->id] = 1; //NPP diz que a task que pegar o recurso não deve ser preemptada, ou seja, recebe prioridade máxima
}

void sem_post(sem_t *sem){
	__disable_irq();
	if(sem->cont < sem->max_cont){
		sem->cont++;
		priorities[OS_curr->id] = sem->prior;
	}
	__enable_irq();
}
/*FIM DA DEFINIÇÃO DOS SEMÁFOROS*/

void OSThread_start(
    OSThread *me,
    uint8_t id, /* thread priority */
    OSThreadHandler threadHandler,
    void *stkSto, uint32_t stkSize, uint8_t tipo)
{
    /* round down the stack top to the 8-byte boundary
    * NOTE: ARM Cortex-M stack grows down from hi -> low memory
    */
    uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
    uint32_t *stk_limit;

    /* priority must be in ragne
    * and the priority level must be unused
    */
    Q_REQUIRE((id < Q_DIM(OS_thread))
              && (OS_thread[id] == (OSThread *)0));

    *(--sp) = (1U << 24);  /* xPSR */
    *(--sp) = (uint32_t)threadHandler; /* PC */
    *(--sp) = 0x0000000EU; /* LR  */
    *(--sp) = 0x0000000CU; /* R12 */
    *(--sp) = 0x00000003U; /* R3  */
    *(--sp) = 0x00000002U; /* R2  */
    *(--sp) = 0x00000001U; /* R1  */
    *(--sp) = 0x00000000U; /* R0  */
    /* additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU; /* R11 */
    *(--sp) = 0x0000000AU; /* R10 */
    *(--sp) = 0x00000009U; /* R9 */
    *(--sp) = 0x00000008U; /* R8 */
    *(--sp) = 0x00000007U; /* R7 */
    *(--sp) = 0x00000006U; /* R6 */
    *(--sp) = 0x00000005U; /* R5 */
    *(--sp) = 0x00000004U; /* R4 */

    /* save the top of the stack in the thread's attibute */
    me->sp = sp;

    /* round up the bottom of the stack to the 8-byte boundary */
    stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

    /* pre-fill the unused part of the stack with 0xDEADBEEF */
    for (sp = sp - 1U; sp >= stk_limit; --sp) {
        *sp = 0xDEADBEEFU;
    }

    if(tipo == 1){
        OS_thread[id] = me;
        me->id = id;
        me->tipo = tipo;
        /* make the thread ready to run */
        if (id > 0U) {
            uint8_t bit = (1U << (id - 1U));
            OS_readySet   |= bit;  /* insert to set */
        }
	}

    if(tipo == 2){
        OS_thread[id] = me;
        me->id = id;
        me->tipo = tipo;
        if (id > 0U) {
            uint8_t bit = (1U << (id - 1U));
            OS_readySet   |= bit;
        }
	}
}

__attribute__ ((naked, optimize("-fno-stack-protector")))
void PendSV_Handler(void) {
__asm volatile (

    /* __disable_irq(); */
    "  CPSID         I                 \n"

    /* if (OS_curr != (OSThread *)0) { */
    "  LDR           r1,=OS_curr       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  CBZ           r1,PendSV_restore \n"

    /*     push registers r4-r11 on the stack */
    "  PUSH          {r4-r11}          \n"

    /*     OS_curr->sp = sp; */
    "  LDR           r1,=OS_curr       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  STR           sp,[r1,#0x00]     \n"
    /* } */

    "PendSV_restore:                   \n"
    /* sp = OS_next->sp; */
    "  LDR           r1,=OS_next       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           sp,[r1,#0x00]     \n"

    /* OS_curr = OS_next; */
    "  LDR           r1,=OS_next       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           r2,=OS_curr       \n"
    "  STR           r1,[r2,#0x00]     \n"

    /* pop registers r4-r11 */
    "  POP           {r4-r11}          \n"

    /* __enable_irq(); */
    "  CPSIE         I                 \n"

    /* return to the next thread */
    "  BX            lr                \n"
    );
}
