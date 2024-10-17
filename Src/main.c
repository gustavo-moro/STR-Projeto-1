#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "miros.h"

uint32_t stack_idleThread[40];

const uint8_t BUFFER_SIZE = 64;

const uint8_t NUM_P = 2;
const uint8_t NUM_C = 1;
const uint8_t empty_init = BUFFER_SIZE/2;
const uint8_t full_init = 32;

const uint32_t STACK_SIZE = 512U;

typedef struct{
	int buffer[64];
	int count;
} BufferFila;

BufferFila fila;

sem_t empty;
sem_t full;
sem_t mutex;

void buffer_init(BufferFila *b){
	b->count = full_init;
}

void buffer_add(BufferFila *b, int item){
	b->buffer[b->count] = item;
	b->count++;
}

void buffer_remove(BufferFila *b){
	for(int i = 0; i < b->count-1; i++){
		b->buffer[i] = b->buffer[i+1];
	}
	b->count--;
}

void produtor(){
	while(1){
		OS_delay((rand() % 100) + 1);
		sem_wait(&empty);
		sem_wait(&mutex);
		buffer_add(&fila, 1);
		sem_post(&mutex);
		sem_post(&full);
	}
}

void consumidor(){
	while(1){
		OS_delay((rand() % 100) + 1);
		sem_wait(&full);
		sem_wait(&mutex);
		buffer_remove(&fila);
		sem_post(&mutex);
		sem_post(&empty);
	}
}

int main() {
	srand(time(NULL));

    OS_init(stack_idleThread, sizeof(stack_idleThread));

    buffer_init(&fila);

    sem_init(&empty, empty_init, BUFFER_SIZE);
    sem_init(&full, full_init, BUFFER_SIZE);
    sem_init(&mutex, 1, 1);

    OSThread produtores[NUM_P];
	OSThread consumidores[NUM_C];

	uint32_t pstack[NUM_P][STACK_SIZE];
	uint32_t cstack[NUM_C][STACK_SIZE];

	for(int i = 0; i < NUM_P; i++){
		OSThread_start(&(produtores[i]), (2*i+1), &produtor, pstack[i], sizeof(pstack[i]));
	}

	for(int i = 0; i < NUM_C; i++){
		OSThread_start(&(consumidores[i]), (2*i+2), &consumidor, cstack[i], sizeof(cstack[i]));
	}

    /* transfer control to the RTOS to run the threads */
    OS_run();

    //return 0;
}
