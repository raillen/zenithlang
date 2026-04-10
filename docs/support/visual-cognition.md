# Cognição Visual e Zenith

Zenith continua perseguindo a mesma meta central: reduzir a carga mental de leitura sem sacrificar expressividade.

## 1. Crescimento vertical

Blocos com `end`, `match`, `attempt/rescue` e encadeamento UFCS favorecem leitura de cima para baixo.

```zt
var total: int = inicial
    .step1(5)
    .step2()
```

Isso reduz o “zig-zag visual” comum em sintaxes muito horizontais.

## 2. Palavras-chave no lugar certo

Zenith prefere `and`, `or`, `not`, `match`, `trait`, `apply`, `where` e `check` em vez de se apoiar só em símbolos curtos e ambíguos.

O benefício é simples: o código vira texto técnico legível, não uma sequência densa de pontuação.

## 3. Erro explícito e localizado

O compilador usa códigos como `ZT-L001`, `ZT-P001` e `ZT-S100`, o que ajuda a localizar a categoria do problema sem obrigar o usuário a adivinhar se a falha é léxica, sintática ou semântica.

## 4. Modelagem visível

Recursos como `type`, `union`, `Optional`, `Outcome` e contratos `where` deixam a intenção do código aparente na superfície.

```zt
struct Item
    pub quantidade: int where it >= 0
end
```

## 5. Menos magia implícita

Zenith tenta evitar atalhos obscuros. Quando algo é especial, ele costuma aparecer no texto:

- `async` e `await`
- `attempt` e `rescue`
- `native lua`
- `apply Trait to Struct`

## 6. Áreas ainda em estabilização

O repositório atual já inclui reatividade (`state`, `computed`, `watch`) e execução assíncrona, mas essas camadas ainda passam por refinamento de ergonomia no runtime Lua. A filosofia permanece a mesma: explicitar o comportamento em vez de escondê-lo.
