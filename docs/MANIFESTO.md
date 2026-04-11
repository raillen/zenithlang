# Zenith Manifesto: Soberania e Acessibilidade Cognitiva

Este documento define a identidade, a filosofia tecnica e a visao estrategica da linguagem Zenith. Zenith nao e apenas uma ferramenta de programacao; e uma infraestrutura cognitiva projetada para transformar a complexidade em clareza.

## 1. Missao Pessoal e Inclusao

Zenith nasceu da necessidade de estudar compiladores e tooling com profundidade, sem transformar cada passo em uma luta contra a propria ferramenta. Como um projeto desenhado por e para pessoas com TDAH e dislexia, o Zenith assume o compromisso de que clareza, previsibilidade e acolhimento sao fundamentais para a experiencia de programar.

Acessibilidade nao e um detalhe; e a base. O Zenith prova que mentes neurodivergentes podem construir sistemas tecnicos e ambiciosos quando o ambiente reduz o ruido e respeita o ritmo de processamento humano.

---

## 2. Tese Central: Legibilidade como Infraestrutura

A maioria das linguagens modernas sacrifica a clareza em favor da concisao extrema. Zenith inverte essa logica atraves de tres principios:

*   **Baixo Atrito Neural**: O programador deve gastar energia resolvendo o problema, nao decifrando a sintaxe. Zenith utiliza palavras-chave explicitas (como end, match, where) para garantir um fluxo visual vertical e previsivel.
*   **Estabilidade Visual**: Simbolos matematicos e operadores aparecem apenas onde sao semanticamente necessarios. A gramatica permanece predominantemente textual e legivel para evitar a "danca de letras" tipica da dislexia.
*   **Explicito sobre Implicito**: Comportamentos magicos sao evitados. Tipos sao nomeados, falhas sao tratadas via Optional e Outcome, e contratos de dados sao declarados via where.

---

## 3. A Trindade Tecnica (v0.3.1)

A fundacao tecnica do Zenith repousa sobre tres pilares de engenharia:

1.  **Soberania Nativa (Ascension)**: O compilador e escrito em Zenith para Zenith. Isso garante independencia total de motores externos e permite otimizações que exploram o maximo do hardware.
2.  **Abstracoes de Custo Zero**: Funcionalidades de alto nivel (Traits, Enums Genericos, Tipagem Estatica) sao resolvidas em tempo de compilacao. O codigo gerado e tao eficiente quanto codigo escrito a mao por especialistas.
3.  **Seguranca por Padrao**: Operacoes de I/O, rede e parsing utilizam o sistema de Outcome. O compilador impede que falhas comuns de runtime cheguem a producao.

---

## 4. Ecossistema e Frameworks Oficiais

O ecossistema Zenith expande o nucleo generico atraves de ferramentas especializadas:

### Dominio Visual e Interfaces
*   **Meridian**: Framework declarativo e reativo para construcao de interfaces graficas (GUI).
*   **Borealis**: Motor de jogo 2D focado em performance industrial, construido sobre Prism via FFI.

### Inteligencia de Dados e Web
*   **Atlas**: Framework de banco de dados tipo-seguro (ORM) com suporte a SQLite e PostgreSQL.
*   **Intersect**: Framework web MVVM para o navegador, permitindo Zenith nativo no browser.
*   **Orion**: Servidor HTTP assíncrono para APIs e serviços de alta performance.

### Ferramental e Inteligencia
*   **Forge**: Editor visual e ambiente de autoria para o ecossistema.
*   **Compass**: Servidor de linguagem (LSP) oficial que fornece inteligencia de codigo.

---

## 5. Estrategia de Distribuicao

O Zenith garante a entrega de software sem friccao tecnica:

*   **Modo Semente (Standalone)**: Gera um executavel de arquivo unico contendo a VM e o codigo otimizado.
*   **Modo Colonia (Bundle)**: Gera uma estrutura de diretorios organizada (/dist) com todas as dependencias nativas gerenciadas automaticamente.

---
*Zenith: Onde a logica encontra o ritmo.*
