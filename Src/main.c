#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "miros.h"

uint32_t stack_idleThread[40];

int a = 0;
int b = 0;
int c = 0;
int d = 0;

sem_t Crit_Reg_A;
sem_t Crit_Reg_B;

int passar_tempo = 0;

void task1(){
	while(1){
		a+=1;
		sem_wait(&Crit_Reg_A);
		for(int i = 0; i < 100; i++){
			passar_tempo++;
		}
		sem_post(&Crit_Reg_A);
		wait_next_period();
	}
}
void task2(){
	while(1){
		b+=1;
		sem_wait(&Crit_Reg_B);
		for(int i = 0; i < 100; i++){
			passar_tempo++;
		}
		sem_post(&Crit_Reg_B);
		wait_next_period();
	}
}
void task3(){
	while(1){
		c+=1;
		sem_wait(&Crit_Reg_A);
		sem_wait(&Crit_Reg_B);
		for(int i = 0; i < 100; i++){
			passar_tempo++;
		}
		sem_post(&Crit_Reg_B);
		sem_post(&Crit_Reg_A);
		wait_next_period();
	}
}
void aperiodic_task(){
	while(1){
		d+=1;
		wait_next_period();
	}
}

OSThread thread_task1;
OSThread thread_task2;
OSThread thread_task3;


OSThread thread_aperiodic_task;

int main() {

    OS_init(stack_idleThread, sizeof(stack_idleThread));

	sem_init(&Crit_Reg_A, 1, 1);
	sem_init(&Crit_Reg_B, 1, 1);

	thread_task1.period = 600;
	thread_task2.period = 800;
	thread_task3.period = 1200;

	thread_aperiodic_task.period = 0; // tarefa aperiódica não tem período

	OSThread_start(&thread_task1, 1, &task1, thread_task1.stack_thread, sizeof(thread_task1.stack_thread), 1);
	OSThread_start(&thread_task2, 2, &task2, thread_task2.stack_thread, sizeof(thread_task2.stack_thread), 1);
	OSThread_start(&thread_task3, 3, &task3, thread_task3.stack_thread, sizeof(thread_task3.stack_thread), 1);

	OSThread_start(&thread_aperiodic_task, 4, &aperiodic_task, thread_aperiodic_task.stack_thread, sizeof(thread_aperiodic_task.stack_thread), 2);

    /* transfer control to the RTOS to run the threads */
    OS_run();

    //return 0;
}
