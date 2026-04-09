# Zenith Cheat Sheet (v0.2)

Referência rápida para os comandos e sintaxes mais comuns.

## Variáveis e Tipos
```zt
var x: int = 10         -- Variável
const Y: text = "Fixo"  -- Constante
var u: int | text = 5   -- União de tipos
```

## Estrutura de Fluxo
```zt
if x > 0
    print("Positivo")
end

match valor
    case 10: print("Dez")
    else: print("Outro")
end

repeat 5 times
    print("Zenith!")
end
```

## Funções e Blocos
```zt
func soma(a: int, b: int) -> int
    return a + b
end

async func carregar()
    await download()
end
```

## Composição (LEGO Style)
```zt
struct Player
    pub nome: text
    pub vida: int = 100
end

trait Voador
    pub func voar()
end

apply Voador to Player
```

## Reatividade
```zt
state energia: int = 100
watch energia
    if it < 20 then print("Cuidado!") end
end
```
