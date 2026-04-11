# Guía de Sintaxis: El Ritmo del Código

Esta guía detalla la estructura del lenguaje Zenith v0.3.1. Cada recurso se presenta con su Regla de Oro para garantizar la menor fricción cognitiva posible.

## 1. Fundamentos y Declaraciones

### Variables y Constantes
Zenith separa explícitamente lo que puede cambiar de lo que es fijo.

*   **Regla**: Usa `var` para datos mutables y `const` para valores fijos en tiempo de compilación.
*   **Contraste**: A diferencia de Lua, donde todo es mutable por defecto, Zenith protege tu estado.

```zt
var nivel: int = 1
const VERSION: text = "0.3.1"
```

### Palabras Clave de Visibilidad
*   `pub`: Hace un símbolo visible fuera del archivo actual.
*   `global`: Declara en el ámbito de la VM (úselo con extrema cautela).

---

## 2. Colecciones de Datos

Zenith ofrece tres estructuras fundamentales para organizar la información.

| Colección | Tipo | Uso Recomendado |
| :--- | :--- | :--- |
| `list<T>` | Dinámico | Secuencias ordenadas de elementos. |
| `map<K, V>` | Hash | Diccionarios de clave y valor. |
| `grid<T>` | Estático 2D | Matrices de alto rendimiento para juegos/gráficos. |

### Ejemplo de Uso
```zt
var items: list<text> = ["Espada", "Escudo"]
var config: map<text, int> = { "volumen": 80 }
var mapa: grid<int> = collections.new_grid(10, 10, 0)
```

---

## 3. Control de Flujo

### Condicionales (`if`)
Zenith utiliza palabras clave textuales para evitar el ruido de paréntesis excesivos.

```zt
if energia > 80
    print("Sistemas OK")
elif energia > 20
    print("Energía Baja")
else
    print("Fallo Crítico")
end
```

### El Oráculo de Decisión (`match`)
El `match` sustituye cadenas largas de `if/else` con claridad visual.

*   **Regla de Oro**: Usa `=>` para respuestas de una línea. Usa `:` para bloques de código complejos.

```zt
match estado
    case "activo" => print("En línea")
    case "error":
        log.error("Fallo detectado")
        intentar_recuperar()
    case _ => print("Desconocido")
end
```

---

## 4. Tratamiento de Errores y Seguridad

Zenith elimina el miedo a los fallos a través de estructuras explícitas.

### El Comando `check`
Valida una condición. Si falla, ejecuta el bloque `else` o interrumpe el flujo.

```zt
-- Como Sentencia
check edad >= 18 else
    return Failure("Edad insuficiente")
end

-- Como Expresión (Propagación)
var archivo = fs.read_text_file("save.dat")?
```

### Bloques `attempt / rescue`
Para capturar errores lanzados por funciones externas o fallos graves.

```zt
attempt
    procesar_datos()
rescue error
    print("Error capturado: " + error)
end
```

---

## 5. Funciones y Modelado

### Definición de Funciones
La flecha `->` apunta al resultado del procesamiento.

```zt
func calcular_dao(fuerza: int, arma: int) -> int
    return fuerza + arma
end
```

### Estructuras (`struct`) e Interfaces (`trait`)
Zenith prefiere Composición sobre Herencia.

```zt
struct Player
    pub nombre: text
    pub hp: int where it >= 0
end

trait Atacante
    func atacar(objetivo: Player)
end

apply Atacante for Player
    func atacar(objetivo: Player)
        objetivo.hp -= 10
    end
end
```

---
*Especificación Zenith v0.3.1 - Manual de Sintaxis*
