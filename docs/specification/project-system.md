# Especificação Técnica: Sistema de Projeto e Módulos

O Zenith utiliza um sistema de módulos Soberano para gerenciar a organização do código e as dependências entre arquivos.

## 1. O Manifesto do Projeto (zenith.ztproj)

Todo projeto Zenith industrial deve conter um arquivo zenith.ztproj na raiz. Este arquivo define os metadados e as regras de build.

### Estrutura do Arquivo
```ztproj
project meu_projeto
    version: 0.1.0
    author:  "Autor"
end

dependencies
    std: official
    extra_lib: "https://github.com/user/lib.git"
end

scripts
    test: "zt test"
    run:  "lua out.lua"
end
```

---

## 2. Namespaces e Arquivos

O comando namespace define o endereço lógico de um arquivo.

- Um arquivo em src/utils/math.zt deve declarar namespace utils.math.
- O compilador utiliza essa declaração para validar se o arquivo está no local correto e para resolver imports.

---

## 3. Sistema de Importação

A palavra-chave import traz símbolos de outros módulos para o escopo atual.

### Tipos de Import
- Import de Módulo: import std.fs (importa o namespace completo).
- Import com Alias: import std.time as t.
- Import de Caminho: import "meu_script" (para arquivos locais fora da estrutura de namespace padrão).

---

## 4. O Zenith Package Manager (ZPM)

ZPM é a ferramenta oficial para:
1.  Sincronização: Baixar dependências Git listadas no manifesto.
2.  Locking: Gerar o arquivo zenith.lock com os hashes exatos dos commits para builds reprodutíveis.
3.  Execução: Rodar scripts customizados definidos no manifesto.

---

## 5. Zenith Doc-Linking (ZDoc)

Zenith permite o desacoplamento total entre a lógica do código e a sua documentação explicativa através do sistema ZDoc.

### Arquivos .zdoc
- Para cada arquivo `.zt`, o compilador busca automaticamente um arquivo correspondente com a extensão `.zdoc` no mesmo diretório.
- Arquivos `.zdoc` contêm apenas blocos de documentação delimitados por `---`.

### A Tag @target
Para associar um comentário a um símbolo específico (função, struct, trait), utiliza-se a tag `@target: nome_do_simbolo`.

Exemplo em `utils.zdoc`:
```zenith
--- @target: validar_email
Verifica se uma string segue o padrão RFC de e-mails.
@param email: Texto a ser validado.
@return: true se for válido.
---
```

O compilador Ascension e o Compass (LSP) mesclam essas informações em tempo de build, garantindo que o código-fonte `.zt` permaneça limpo e focado na implementação.

---
*Zenith Specification v0.3.0*
