# CLI Reference

> Referencia publica de CLI e diagnosticos.
> Audience: user, contributor
> Surface: reference
> Status: current

## Paginas

| Pagina | Uso |
| --- | --- |
| `zt.md` | comandos do compilador/driver `zt` |
| `zpm.md` | comandos do package manager `zpm` |
| `diagnostics.md` | onde consultar codigos e formato de diagnosticos |

## Binarios

- Windows: `zt.exe`, `zpm.exe`, `zt-lsp.exe`.
- Linux/macOS: nomes equivalentes sem `.exe`, quando gerados.

## Validacao rapida

```powershell
.\zt.exe help
.\zpm.exe help
python tools/check_docs_paths.py
```
