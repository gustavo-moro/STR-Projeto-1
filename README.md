Aluno: Gustavo Moro
Matrícula: 22101929
---
**Implementação dos Semáforos**

Os semáforos são utilizados para garantir que os produtores e consumidores acessem o buffer de forma sincronizada, evitando condições de corrida. No projeto, foram usados três semáforos:

    -empty: Controla o número de espaços livres no buffer.
    -full: Controla o número de itens disponíveis no buffer.
    -mutex: Garante acesso exclusivo ao buffer, evitando que múltiplas threads alterem o estado ao mesmo tempo.

**Funções de Semáforo**

	-sem_init: Inicializa o semáforo com um valor inicial e o valor máximo.
    -sem_wait: Espera até que o semáforo tenha um valor positivo, indicando que a operação pode prosseguir.
    -sem_post: Incrementa o valor do semáforo, liberando a operação para outra thread.

**Implementação do Produtor e Consumidor**

O sistema possui duas threads de produtores e uma thread de consumidor. Os produtores adicionam itens ao buffer, enquanto o consumidor remove itens. A sincronização é realizada através dos semáforos.

**Produtor**

A thread do produtor insere um item no buffer, caso haja espaço disponível, e notifica o semáforo full após a inserção.

**Consumidor**

A thread do consumidor remove um item do buffer, caso existam itens disponíveis, e notifica o semáforo empty após a remoção.

Ambas as threads usam o semáforo mutex para garantir que apenas uma thread acesse o buffer por vez.