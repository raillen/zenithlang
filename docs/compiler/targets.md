# Alvos de Compilação

> Status: `Current`

Hoje o Zenith j? consegue se compilar, se otimizar e se distribuir para plataformas modernas. O backend principal continua dialogando com a história técnica da linguagem, mas distribuição multiplataforma deixou de ser promessa e virou capacidade presente.

## O que j? e atual

### Backend principal

O pipeline atual continua valorizando o caminho que amadureceu Zenith at? aqui:

- codegen consolidado
- portabilidade pratica
- integração madura com o ecossistema existente
- boa base para tooling, automação e jogos

### Distribuição multiplataforma

Ascension j? entrega uma camada real de distribuição moderna, com recursos como:

- suporte a `@windows`, `@linux` e `@macos`
- modos standalone e bundle
- integração com FFI para C e C++
- empacotamento mais direto para uso real

## O que continua como horizonte técnico

Ainda faz sentido manter algumas direções como visão de longo prazo:

- compilação direta para C
- bytecode ou IR mais autonoma
- novos backends nativos
- uma camada de distribuição ainda mais independente

## Regra editorial para falar de targets

Sempre diferenciar:

- `Atual`: o que Zenith entrega hoje
- `Concluído`: o que j? faz parte do motor Ascension oficial
- `Visão`: o que continua como expansão futura