# Roadmap de Bibliotecas: Zenith Stdlib

Este documento define a estratégia, a filosofia e o cronograma de implementação dos módulos oficiais da linguagem Zenith.

## 1. Filosofia da Stdlib

1.  Segurança por Padrão: Operações que podem falhar (IO, Network, Parsing) devem retornar Outcome<T, E>.
2.  Zenith-First: Sempre que possível, implementar a lógica em Zenith. Usar native lua apenas para acesso ao sistema operacional ou performance crítica.
3.  API Fluida: Nomes de funções verbais e verticalidade (legibilidade acima de concisão extrema).
4.  Baterias Inclusas: Foco nos domínios de Automação, Jogos e UI.

---

## 2. Fase 1: Fundação (v0.2.5) - Concluído

Foco: Permitir que o Zenith interaja com o mundo real e manipule dados básicos.

- [X] std.fs (File System)
- [X] std.os (Operating System)
- [X] std.json (Dados estruturados)

---

## 3. Fase 2: Utilidades e Tempo (v0.2.6) - Concluído

Foco: Controle de fluxo temporal e manipulação avançada de texto.

- [X] std.time
- [X] std.text

---

## 4. Fase 3: Tooling e Ecossistema (v0.2.7) - Concluído

Foco: Gestão de pacotes e experiência do desenvolvedor.

- [X] ZPM (Zenith Package Manager)

---

## 5. Fase 4: Avançado e Conectividade (v0.2.8) - Concluído

Foco: Poder computacional, segurança e comunicação.

- [X] std.math
- [X] std.collections
- [X] std.crypto: Base64, Hashes (MD5, SHA256), HMAC, UUID v4.
- [X] std.log: Níveis, Cores Aura, Sinks e Filtros.
- [X] std.net (Baixo Nível)
- [X] std.http (Alto Nível)
- [X] std.reflect
- [X] std.events
- [X] std.os.process

---

## 6. Critérios de Aceite para um Módulo

- [X] Testes de integração cobrindo casos de sucesso e erro via ZTest.
- [X] Documentação técnica exaustiva em docs/api/.
- [X] Zero vazamento de globais Lua (todo native lua encapsulado).

---
*Atualizado em: 11 de Abril de 2026*
