# Zenith パッケージ マネージャー (ZPM) ユーザー ガイド

> ZPM 公開ガイド。
> 対象者: ユーザー、パッケージ作成者
> 表面: 公開
> ステータス: 現在

＃＃ 客観的

ZPM は Zenith 言語パッケージ マネージャーです。

現在のカットではバイナリとして存在します`zpm.exe`ドライバー統合モデルにも準拠しています`zt`。

## クイックヘルプ

```powershell
.\zpm.exe help
```

## プロジェクトを作成する

```powershell
.\zpm.exe init my_project
```

これにより、プロジェクトが作成されます`zenith.ztproj`。

## 依存関係を追加する

プロジェクト フォルダーを入力します。

```powershell
cd my_project
```

パッケージを追加します。

```powershell
..\zpm.exe add borealis@0.1.0
```

マニフェストは依存関係を受け取ります`[dependencies]`。

## 依存関係をインストールする

```powershell
..\zpm.exe install
```

これにより依存関係が解決され、`zenith.lock`。

## 依存関係をリストする

```powershell
..\zpm.exe list
```

## 依存関係を更新する

```powershell
..\zpm.exe update
```

または、特定のパッケージを更新します。

```powershell
..\zpm.exe update borealis
```

## 依存関係を削除する

```powershell
..\zpm.exe remove borealis
```

## マニフェスト スクリプトを実行する

```powershell
..\zpm.exe run build
```

＃＃ 公開

```powershell
..\zpm.exe publish
```

使用`publish`パッケージを共有する前の検証として。

## 基本的なマニフェスト

```toml
[project]
name = "my_app"
kind = "app"
version = "0.1.0"

[dependencies]
borealis = "0.1.0"
```

## ロックファイル

`zenith.lock`プロジェクトに再現性が必要な場合は、バージョン管理する必要があります。

## 電流カットの制限

- パッケージ エコシステムはまだアルファ版です。
- 公式パッケージは以前に変更される可能性があります`1.0.0`。
- 使用`zt check`後`zpm install`実際のプロジェクトを検証します。