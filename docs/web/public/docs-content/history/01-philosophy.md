# Origem e Filosofia do Zenith

Zenith nasceu da ideia de que legibilidade não é luxo: é infraestrutura cognitiva.

## Tese central

Muitas linguagens modernas concentram demais a informação em símbolos, abreviações e convenções implícitas. Zenith segue outro caminho:

- fluxo vertical
- palavras-chave visíveis
- tipagem explícita
- erros pedagógicos
- modelagem com intenção aparente

## Pilares do projeto

### 1. Baixo atrito neural

Quem lê o código não deveria gastar energia descobrindo onde um bloco termina ou qual atalho oculto o compilador aplicou.

Por isso Zenith prefere:

- `end`
- `match`
- `attempt/rescue`
- `check`
- `where`

### 2. Estabilidade visual

O objetivo não é remover símbolos, e sim fazer com que eles apareçam só quando ajudam de fato. `@campo`, `..`, `?` e `!` existem, mas dentro de uma gramática que continua predominantemente textual.

### 3. Explícito sobre implícito

Essa filosofia aparece em toda a linguagem:

- tipos nomeados com `var x: int`
- falhas explícitas com `Optional` e `Outcome`
- contratos em dados com `where`
- composição declarada com `trait` e `apply`

### 4. Aplicação prática

Zenith não busca apenas “ser bonito”. O design visa tarefas reais como:

- automação
- tooling
- desktop
- scripts de sistema
- projetos educacionais

## Como isso aparece no código

```zt
struct Player
    pub nome: text
    pub vida: int where it >= 0
end

check player.vida > 0 else
    panic("jogador inválido")
end
```

O leitor enxerga a forma do dado, a regra do dado e a falha possível sem precisar sair do trecho.

## Estado atual

A filosofia permanece estável mesmo com a expansão recente da stdlib. O projeto segue preferindo clareza de surface syntax, contratos explícitos e APIs pequenas e nomeadas de forma direta.
