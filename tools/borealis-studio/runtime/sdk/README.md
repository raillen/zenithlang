# Borealis SDK placeholder

Esta pasta representa o SDK que deve ser empacotado junto do Borealis Studio.

Nao copie o codigo fonte completo do repositorio da linguagem para ca.
O conteudo esperado em uma release e o runtime versionado:

```text
runtime/sdk/
  bin/
    zt.exe
    borealis-preview.exe
  stdlib/
  preview/
    zenith.ztproj
  templates/
```

Durante desenvolvimento, o Studio ainda usa o repositorio `zenith-lang-v2` como fallback.
Em release, defina `BOREALIS_SDK_ROOT` ou entregue esta pasta preenchida.

Para preencher esta pasta no workspace local:

```powershell
cd tools/borealis-studio
npm run sdk:assemble
```

Os arquivos gerados aqui sao ignorados pelo Git. O script valida o projeto de
preview e o package Borealis com o `zt.exe` copiado para `runtime/sdk/bin`.
