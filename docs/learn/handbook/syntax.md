# Guia de Sintaxe: O Ritmo do Código

Este guia detalha a estrutura da linguagem Zenith v0.3.1. Cada recurso é apresentado com sua Regra de Ouro para garantir o menor atrito cognitivo possível.

## 1. Fundamentos e Declarações

### Variáveis e Constantes
Zenith separa explicitamente o que pode mudar do que é fixo.

*   **Regra**: Use `var` para dados mutáveis e `const` para valores fixos em tempo de compilação.
*   **Contraste**: Diferente do Lua, onde tudo é mutável por padrão, Zenith protege seu estado.

```zt
var nivel: int = 1
const VERSAO: text = "0.3.1"
```

### Palavras-Chave de Visibilidade
*   `pub`: Torna um símbolo visível fora do arquivo atual.
*   `global`: Declara no escopo da VM (use com extrema cautela).

---

## 2. Coleções de Dados

Zenith oferece três estruturas fundamentais para organizar informações.

| Coleção | Tipo | Uso Recomendado |
| :--- | :--- | :--- |
| `list<T>` | Dinâmico | Sequências ordenadas de itens. |
| `map<K, V>` | Hash | Dicionários de chave e valor. |
| `grid<T>` | Estático 2D | Matrizes de alta performance para jogos/gráficos. |

### Exemplo de Uso
```zt
var itens: list<text> = ["Espada", "Escudo"]
var config: map<text, int> = { "volume": 80 }
var mapa: grid<int> = collections.new_grid(10, 10, 0)
```

---

## 3. Controle de Fluxo

### Condicionais (`if`)
Zenith utiliza palavras-chave textuais para evitar o ruído de parênteses excessivos.

```zt
if energia > 80
    print("Sistemas OK")
elif energia > 20
    print("Energia Baixa")
else
    print("Falha Crítica")
end
```

### O Oráculo de Decisão (`match`)
O `match` substitui cadeias longas de `if/else` com clareza visual.

*   **Regra de Ouro**: Use `=>` para respostas de uma linha. Use `:` para blocos de código complexos.

```zt
match status
    case "ativo" => print("Online")
    case "erro":
        log.error("Falha detectada")
        tentar_recuperar()
    case _ => print("Desconhecido")
end
```

---

## 4. Tratamento de Erros e Segurança

Zenith elimina o medo de falhas através de estruturas explícitas.

### O Comando `check`
Valida uma condição. Se falhar, executa o bloco `else` ou interrompe o fluxo.

```zt
-- Como Statement
check idade >= 18 else
    return Failure("Idade insuficiente")
end

-- Como Expressão (Propagação)
var arquivo = fs.read_text_file("save.dat")?
```

### Blocos `attempt / rescue`
Para capturar erros lançados por funções externas ou falhas graves.

```zt
attempt
    processar_dados()
rescue erro
    print("Erro capturado: " + erro)
end
```

---

## 5. Funções e Modelagem

### Definição de Funções
A seta `->` aponta para o resultado do processamento.

```zt
func calcular_dano(forca: int, arma: int) -> int
    return forca + arma
end
```

### Estruturas (`struct`) e Contratos (`trait`)
Zenith prefere Composição sobre Herança.

```zt
struct Player
    pub nome: text
    pub hp: int where it >= 0
end

trait Atacante
    func atacar(alvo: Player)
end

apply Atacante for Player
    func atacar(alvo: Player)
        alvo.hp -= 10
    end
end
```

---
*Zenith Specification v0.3.1 - Manual de Sintaxe*
