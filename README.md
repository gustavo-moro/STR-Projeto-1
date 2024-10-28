Aluno: Gustavo Moro
Matrícula: 22101929
---
#Projeto 2 - Tarefa periódica e Escalonador

    -O algoritmo de escalonamento escolhido foi o Rate Monotonic (RM), que prioriza tarefas com menor período, permitindo que tarefas mais frequentes tenham maior prioridade de execução. Este projeto implementa esse escalonamento em um ambiente de RTOS usando o Miros.

##Estrutura do código

    -Funções task: No arquivo main.c, as funções task são declaradas e utilizadas pelas threads. Cada task representa uma função genérica executada pelas threads criadas na main

    -Configuração de Tarefas no main(): As tarefas (task1, task2, task3) são configuradas com valores de exec_time (tempo de execução) e period (período)

    -Struct OSThread: A struct foi alterada para conter as informações necessárias para as tarefas periódicas
        -exec_time: Tempo de execução da tarefa, que representa quanto tempo a tarefa precisa para ser concluída.
        -exec_time_counter: Tempo de execução váriavel, que chega a 0 quando a task termina de ser executada e é setada para exec_time a cada release
        -period: Período da tarefa, determinando a frequência com que a tarefa deve ser executada.
        -last_release: Armazena o instante de tempo da última execução da tarefa.
        -next_release: Armazena o instante de tempo em que a próxima execução da tarefa deve ocorrer.
        -stack_thread: Pilha dedicada para a execução da tarefa.

    -Função OS_sched: atualiza OS_next com base no RM    
    
    -Função find_next_thread(): escolhe a próxima tarefa a ser executada com base no menor período das tarefas prontas.

    -Vetor thread_states[]: armazena o estado de cada thread/task: 
        -1 indica que a thread está pronta
        -0 indica que a thread não está pronta
    
    -Variável current_time: variável atualizada a cada tick

    -Função OS_tick: atualizações necessárias a todo tick
        -gasta 1 tick do tempo de execução da task que está executando
        -verifica se é hora de liberar cada tarefa
        -caso libere tarefa, atualiza next_release, last_release e exec_time_counter
    