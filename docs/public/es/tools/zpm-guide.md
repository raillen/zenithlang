# Guía del usuario de Zenith Package Manager (ZPM)

> Guía pública ZPM.
> Público: usuario, autor del paquete
> Superficie: pública
> Estado: actual

## Objetivo

ZPM es el administrador de paquetes de idiomas de Zenith.

En el corte actual, existe como binario.`zpm.exe`y también sigue el modelo de integración de controladores.`zt`.

## Ayuda rápida

```powershell
.\zpm.exe help
```

## Crear proyecto

```powershell
.\zpm.exe init my_project
```

Esto crea un proyecto con`zenith.ztproj`.

## Agregar dependencia

Ingrese a la carpeta del proyecto:

```powershell
cd my_project
```

Añade el paquete:

```powershell
..\zpm.exe add borealis@0.1.0
```

El manifiesto recibe la dependencia de`[dependencies]`.

## Instalar dependencias

```powershell
..\zpm.exe install
```

Esto resuelve dependencias y genera`zenith.lock`.

## Listar dependencias

```powershell
..\zpm.exe list
```

## Actualizar dependencias

```powershell
..\zpm.exe update
```

O actualice un paquete específico:

```powershell
..\zpm.exe update borealis
```

## Eliminar dependencia

```powershell
..\zpm.exe remove borealis
```

## Ejecutar script de manifiesto

```powershell
..\zpm.exe run build
```

## Publicar

```powershell
..\zpm.exe publish
```

Usar`publish`como validación antes de compartir un paquete.

## Manifiesto básico

```toml
[project]
name = "my_app"
kind = "app"
version = "0.1.0"

[dependencies]
borealis = "0.1.0"
```

## Archivo de bloqueo

`zenith.lock`Se debe versionar cuando el proyecto necesite reproducibilidad.

## Límites del corte actual

- El ecosistema de paquetes aún está en alfa.
- Los paquetes oficiales pueden cambiar antes`1.0.0`.
- Usar`zt check`después`zpm install`para validar el proyecto real.