# QT Process Scheduler

Projeto desenvolvido em **Qt** para simulação de algoritmos de escalonamento de processos.
Permite visualizar timelines e analisar o comportamento de diferentes estratégias de escalonamento.

## Funcionalidades

* Representação visual da execução dos processos ao longo do tempo.
* Suporte a múltiplos algoritmos de escalonamento (ex.: Round Robin, FCFS, SJF).
* Interface interativa para adicionar e gerenciar processos.

## Estrutura

* `*.pro` : arquivo de projeto Qt
* `*.cpp` / `*.h` : código-fonte
* `*.ui` : arquivos de interface
* `build-*/` : diretórios de build (não versionados)
* `.pro.user` : configurações locais do Qt Creator (atualmente incluídas no repositório)

## Como compilar

1. Abrir o projeto no **Qt Creator**.
2. Configurar o kit de compilação desejado (compilador e versão do Qt).
3. Buildar e executar o projeto.

## Uso

* Adicionar processos com tempo de execução e tempo de chegada.
* Selecionar o algoritmo de escalonamento.
* Executar a simulação para visualizar a timeline e métricas dos processos.

## Disciplina

**Engenharia de Computação – ECP165: Laboratório de Programação III**