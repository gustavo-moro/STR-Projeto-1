Aluno: Gustavo Moro
Matrícula: 22101929
---
#Projeto 3 - Serviço de tarefa aperiódica e protocolo de compartilhamento de recurso

    -Para o projeto foram implementados o background scheduling para lidar com tarefas aperiódicas e o o nom preemptive protocol para gerenciar o compartilhamento de recursos.

##Estrutura do código

    -Funções task: No arquivo main.c, as funções task são declaradas e utilizadas pelas threads. Cada task representa uma função genérica executada pelas threads criadas na main

    -Configuração de Tarefas no main(): 
        -As tarefas (task1, task2, task3) são configuradas com de period (período)
        -A tarefa aperiódica é configurada com período = 0 (não tem período)

    -Struct OSThread: A struct foi alterada para conter as informações necessárias para as tarefas 
        -period: Período da tarefa, determinando a frequência com que a tarefa deve ser executada
        -stack_thread: Pilha dedicada para a execução da tarefa
        -id: identificadorda da thread
        -tipo: 1 para periódicas e 2 para aperiódicas

    -Função fix_priorities(): fixa as prioridades no vetor priorities com base no período    

    -Função OS_sched: atualiza OS_next com base no RM para tarefas periódicas, caso não haja nenhuma tarefa periódica pronta, executa tarefas periódicas (background scheduling)    
    
    -Função sched_aperiodic(): escolhe a próxima tarefa aperiódica a ser executada

    -Função find_next_thread(): escolhe a próxima tarefa periódica a ser executada com base nas prioridades das tarefas prontas.

    -Bitmasks OS_readySet e OS_delayedSet para verificar o estado das tasks
    
    -Variável current_time: variável atualizada a cada tick

    -Função wait_next_period:
        -desabilita a thread até o próximo período para tarefas periódicas
        -para tarefas aperiódicas, desabilita ela por um período aleatório para simular a aperiodicidade

    -Função OS_tick: atualizações necessárias a todo tick
        -decrementa o timeout de cada task e verifica se pode tornar a tarefa pronta novamente

    -sem_wait(): alterado para que a thread que pegar o semáforo tenha prioridade máxima (NPP)

    -sem_post(): volta a prioridade normal   