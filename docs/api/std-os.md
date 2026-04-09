# Módulo `std.os`

O módulo `os` fornece acesso a funcionalidades do sistema operacional, informações de hardware e ambiente.

---

## Propriedades

| Propriedade | Tipo | Descrição |
| :--- | :--- | :--- |
| `version` | `text` | Versão atual da linguagem Zenith. |
| `platform` | `text` | Nome do sistema operacional (ex: "windows", "linux"). |
| `arch` | `text` | Arquitetura do processador (ex: "x64", "arm64"). |
| `project_root` | `text` | Caminho raiz do projeto atual. |

---

## Funções

### `get_env_variable(name: text) -> Optional<text>`
Busca o valor de uma variável de ambiente pelo nome. Retorna `Empty` se não for encontrada.

### `get_all_env_variables() -> map<text, text>`
Retorna um mapa contendo todas as variáveis de ambiente disponíveis para o processo.

### `get_system_info() -> SystemInfo`
Retorna uma struct com detalhes sobre o sistema operacional.

### `get_hardware_info() -> HardwareInfo`
Coleta informações sobre o hardware (CPU, RAM).

### `run_command(cmd: text) -> int`
Executa um comando no shell do sistema e retorna o código de saída.

### `get_special_path(kind: SpecialPath) -> text`
Retorna o caminho para pastas padrão do sistema (Desktop, Documents, etc).

### `get_working_dir() -> text`
Retorna o diretório de trabalho atual (CWD).

### `is_admin() -> bool`
Verifica se o processo tem privilégios elevados (root/administrador).

### `exit(code: int)`
Termina a execução do programa Zenith imediatamente.

---

## Estruturas de Dados

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

### `SpecialPath` (Enum)
- `Config`
- `Temp`
- `Desktop`
- `Documents`
