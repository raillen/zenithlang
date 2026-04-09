# Origem e Filosofia do Zenith

**Objetivo**: Explicar o propósito da linguagem e sua essência.  
**Público-alvo**: Usuários, contribuidores e entusiastas de design de linguagens.  
**Contexto**: Fundação do Projeto (v0.1 e v0.2)

## Conteúdo Principal

O Zenith não nasceu para ser apenas mais uma "camada de açúcar sintático" para o Lua. Ele nasceu de uma observação: muitas linguagens de programação modernas punem o cérebro do desenvolvedor com densidade simbólica excessiva, ambiguidades visuais e constantes trocas de contexto mental.

Para pessoas com **TDAH e Dislexia**, essas fricções não são apenas "detalhes", mas barreiras reais que impedem o fluxo de trabalho.

### Os Pilares da Filosofia Zenith

1. **Baixo Atrito Neural**: A sintaxe deve ser previsível. O cérebro não deve gastar energia tentando descobrir onde um bloco termina ou se um símbolo é um operador ou uma decoração.
2. **Estabilidade Visual**: Evitamos o uso excessivo de símbolos (`{}`, `()`, `[]`) em contextos onde palavras-chave claras (`func`, `end`, `match`) oferecem uma âncora visual mais forte.
3. **Explícito sobre Implícito**: Preferimos que o código diga exatamente o que está acontecendo, mesmo que isso signifique alguns caracteres a mais de digitação.
4. **Foco em Domínios Práticos**: O Zenith é otimizado para o que realmente importa no dia a dia: Jogos, Automação, UI/UX e Ferramentas Desktop.

## Decisões e Justificativas

- **Decisão**: Uso de `end` para fechar todos os blocos.
- **Motivo**: Oferece uma barreira visual clara e uniforme, facilitando a leitura vertical e reduzindo a confusão com chaves aninhadas.
- **Alternativa**: Blocos baseados em indentação (Python-style). Foi descartado por ser frágil em processos de refatoração rápida e menos acessível para leitores de tela e disléxicos.

## Exemplos Comentados

```zt
-- Sintaxe limpa, orientada a palavras-chave
func greet(name: text)
    if name == "Mundo"
        print("Olá, Universo!")
    else
        print("Olá, " + name)
    end
end
```
1. `func` inicia a intenção de forma clara.
2. `name: text` exige o tipo, removendo a dúvida sobre o que a função espera.
3. `end` fecha o bloco de forma inequívoca em qualquer nível de indentação.

## Impactos na Implementação
A filosofia dita que o Lexer e o Parser devem ser **Rígidos mas Amigáveis**. O compilador não tenta "adivinhar" o que você quis dizer; ele aponta o erro de forma pedagógica.

**Status**: Implementado (v1.0-alpha)
