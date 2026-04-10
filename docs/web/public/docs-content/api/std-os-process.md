# Módulo `std.os.process`

O `std.os.process` é a adição mais recente da stdlib. Ele expõe criação de processos em background, captura opcional de saída e espera assíncrona.

## Tipo `Process`

```zt
struct Process
    pub pid: int
    pub executable: text

    pub async func wait() -> int
    pub func kill() -> bool
    pub func read_output() -> Optional<text>
end
```

## Funções

| API | Descrição |
| :-- | :-- |
| `spawn(cmd: text, args: list<text> = [], capture: bool = false, env: map<text, text> = {}) -> Outcome<Process, text>` | Inicia um programa em background. |
| `current_id() -> int` | Retorna o identificador do processo atual. |

## Exemplo

```zt
import std.os.process

pub async func main() -> int
    var proc = process.spawn("echo", ["Olá"], capture: true)

    match proc
        case Success(p) =>
            var handle: any = p
            var code = await handle.wait()
            print("Exit: " + code)
            print("Stdout: " + (handle.read_output() or ""))
        case Failure(err) =>
            print("Falha: " + err)
    end

    return 0
end
```

## Limitações do snapshot atual

- `pid` é um identificador simbólico gerado pelo runtime, não o PID real do sistema.
- `kill()` hoje remove apenas o estado local usado para controle; ele ainda não garante encerramento real do processo no host.
- `current_id()` retorna `0` por enquanto.
- `wait()` depende da infraestrutura async do runtime, que ainda está em estabilização no repositório atual.

## Quando usar

- Tarefas de automação e shell.
- Execução de ferramentas externas.
- Processos que precisam de captura simples de `stdout`.
