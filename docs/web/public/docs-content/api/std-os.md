# Módulo `std.os`

O `std.os` concentra acesso ao ambiente do processo, informações do host e caminhos especiais do sistema.

## Propriedades

| API | Tipo | Descrição |
| :-- | :-- | :-- |
| `version` | `text` | String de versão exposta pela stdlib atual. |
| `platform` | `text` | Plataforma detectada, como `windows` ou `linux`. |
| `arch` | `text` | Arquitetura atual, como `x64`. |
| `project_root` | `text` | Caminho base do projeto atual. |

## Tipos

### `SystemInfo`

```zt
struct SystemInfo
    pub name: text
    pub arch: text
    pub username: text
    pub home_dir: text
end
```

### `HardwareInfo`

```zt
struct HardwareInfo
    pub cpu: text
    pub ram_gb: float
    pub battery: float?
    pub is_charging: bool
end
```

### `SpecialPath`

```zt
enum SpecialPath
    Config, Temp, Desktop, Documents
end
```

## Funções

| API | Descrição |
| :-- | :-- |
| `get_env_variable(name: text) -> Optional<text>` | Lê uma variável de ambiente. |
| `get_all_env_variables() -> map<text, text>` | Retorna o ambiente disponível para o processo. |
| `get_system_info() -> SystemInfo` | Dados básicos do sistema atual. |
| `get_hardware_info() -> HardwareInfo` | CPU, RAM e dados de energia quando disponíveis. |
| `run_command(cmd: text) -> int` | Executa um comando no shell. |
| `get_special_path(kind: SpecialPath) -> text` | Resolve pastas especiais. |
| `get_working_dir() -> text` | Diretório atual. |
| `is_admin() -> bool` | Detecta privilégios elevados. |
| `exit(code: int)` | Encerra o processo atual. |

## Exemplo

```zt
import std.os

pub func main() -> int
    print("Plataforma: " + os.platform)
    print("Usuário: " + (os.get_env_variable("USERNAME") or "desconhecido"))
    print("CWD: " + os.get_working_dir())
    return 0
end
```

## Companheiro: `std.os.process`

Processos em background agora vivem em um submódulo próprio:

```zt
import std.os.process
```

Veja [`std.os.process`](./std-os-process.md) para `spawn`, `wait`, `kill` e captura de saída.

## Observações

- A implementação nativa atual prioriza Windows e Linux com fallbacks simples.
- Alguns dados dependem do host Lua e podem voltar com valores aproximados ou padrões.
