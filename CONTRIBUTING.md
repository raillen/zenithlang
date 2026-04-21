# Contributing to Zenith

Obrigado por contribuir.

Este arquivo cobre 3 pontos:

1. Como enviar contribuicao
2. Licenciamento de contribuicoes
3. Regras de terceiros (codigo copiado/portado)

## 1) Fluxo curto

1. Abra issue (bug/feature) com contexto minimo.
2. Faca branch com foco unico.
3. Adicione teste para bug fix ou feature.
4. Rode os gates do projeto antes de abrir PR.
5. Abra PR com descricao curta e objetiva.

Gate minimo oficial (local):

- `python run_all_tests.py`
- `./zt.exe fmt tests/behavior/tooling_gate_smoke --check`
- `./zt.exe doc check tests/behavior/tooling_gate_smoke`

## 2) Licenca de contribuicoes

Ao enviar codigo para este repositorio, voce concorda que sua contribuicao e licenciada como:

- Apache-2.0 OR MIT

Regra padrao (inspirada no ecossistema Rust):

- A menos que voce declare o contrario explicitamente,
- toda contribuicao submetida intencionalmente para o projeto,
- entra no projeto sob licenca dupla Apache-2.0 OR MIT,
- sem termos adicionais.

## 3) Regras para codigo de terceiros

Antes de copiar/portar codigo de outro projeto:

- confirme compatibilidade de licenca com Apache-2.0 OR MIT;
- preserve avisos de copyright exigidos;
- cite a origem no PR;
- evite qualquer codigo sob licenca incompativel.

Nao enviar:

- codigo GPL que exija relicenciamento do Zenith;
- conteudo sem autorizacao clara;
- codigo com termos adicionais restritivos.

## 4) Sign-off (DCO simples)

Recomendado em commits de contribuicao:

Signed-off-by: Seu Nome <seu-email>

Exemplo de comando:

`git commit -s -m "fix: corrigir parser edge case"`

## 5) Checklist de PR

- [ ] problema esta claro
- [ ] testes adicionados/atualizados
- [ ] documentacao atualizada se houve mudanca de comportamento
- [ ] sem regressao nos comandos de gate
- [ ] origem/licenca de codigo externo validada
