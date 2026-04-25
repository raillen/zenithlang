const cp = require('child_process');
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');

const LANGUAGE = 'zenith';

class CompassClient {
  constructor(context, diagnostics) {
    this.context = context;
    this.diagnostics = diagnostics;
    this.proc = null;
    this.buffer = Buffer.alloc(0);
    this.nextId = 1;
    this.pending = new Map();
    this.started = false;
  }

  resolveLspPath() {
    const configured = vscode.workspace.getConfiguration('zenith').get('lsp.path');
    if (configured && configured.trim().length > 0) return configured;
    const repoRoot = path.resolve(this.context.extensionPath, '..', '..');
    const local = path.join(repoRoot, process.platform === 'win32' ? 'zt-lsp.exe' : 'zt-lsp');
    if (fs.existsSync(local)) return local;
    return process.platform === 'win32' ? 'zt-lsp.exe' : 'zt-lsp';
  }

  async ensureStarted() {
    if (this.started && this.proc && !this.proc.killed) return;
    const exe = this.resolveLspPath();
    this.proc = cp.spawn(exe, [], { cwd: path.dirname(exe), stdio: ['pipe', 'pipe', 'pipe'] });
    this.started = true;
    this.proc.stdout.on('data', (chunk) => this.onData(chunk));
    this.proc.stderr.on('data', (chunk) => console.error(`[zenith-lsp] ${chunk.toString()}`));
    this.proc.on('exit', () => {
      this.started = false;
      for (const [, pending] of this.pending) pending.reject(new Error('Compass LSP exited'));
      this.pending.clear();
    });
    const folder = vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0];
    await this.request('initialize', {
      capabilities: {},
      processId: process.pid,
      rootUri: folder ? folder.uri.toString() : null,
    });
    this.notify('initialized', {});
  }

  stop() {
    if (!this.proc) return;
    try {
      this.request('shutdown', {}).finally(() => this.notify('exit', {}));
    } catch (_) {
      this.proc.kill();
    }
  }

  onData(chunk) {
    this.buffer = Buffer.concat([this.buffer, chunk]);
    while (true) {
      const headerEnd = this.buffer.indexOf('\r\n\r\n');
      if (headerEnd < 0) return;
      const header = this.buffer.slice(0, headerEnd).toString('ascii');
      const match = /content-length:\s*(\d+)/i.exec(header);
      if (!match) {
        this.buffer = this.buffer.slice(headerEnd + 4);
        continue;
      }
      const length = Number(match[1]);
      const bodyStart = headerEnd + 4;
      const bodyEnd = bodyStart + length;
      if (this.buffer.length < bodyEnd) return;
      const body = this.buffer.slice(bodyStart, bodyEnd).toString('utf8');
      this.buffer = this.buffer.slice(bodyEnd);
      this.handleMessage(JSON.parse(body));
    }
  }

  handleMessage(message) {
    if (message.id !== undefined && this.pending.has(message.id)) {
      const pending = this.pending.get(message.id);
      this.pending.delete(message.id);
      if (message.error) pending.reject(new Error(message.error.message || 'LSP request failed'));
      else pending.resolve(message.result);
      return;
    }
    if (message.method === 'textDocument/publishDiagnostics') {
      this.applyDiagnostics(message.params);
    }
  }

  applyDiagnostics(params) {
    if (!params || !params.uri) return;
    const uri = vscode.Uri.parse(params.uri);
    const items = (params.diagnostics || []).map((diag) => {
      const range = new vscode.Range(
        diag.range.start.line,
        diag.range.start.character,
        diag.range.end.line,
        diag.range.end.character,
      );
      const severity = diag.severity === 2
        ? vscode.DiagnosticSeverity.Warning
        : diag.severity === 3
          ? vscode.DiagnosticSeverity.Information
          : diag.severity === 4
            ? vscode.DiagnosticSeverity.Hint
            : vscode.DiagnosticSeverity.Error;
      const item = new vscode.Diagnostic(range, diag.message, severity);
      item.source = diag.source || 'zenith';
      item.code = diag.code;
      return item;
    });
    this.diagnostics.set(uri, items);
  }

  send(payload) {
    if (!this.proc || !this.proc.stdin.writable) return;
    const body = Buffer.from(JSON.stringify(payload), 'utf8');
    const header = Buffer.from(`Content-Length: ${body.length}\r\n\r\n`, 'ascii');
    this.proc.stdin.write(Buffer.concat([header, body]));
  }

  request(method, params) {
    const id = this.nextId++;
    const payload = { jsonrpc: '2.0', id, method, params };
    this.send(payload);
    return new Promise((resolve, reject) => {
      this.pending.set(id, { resolve, reject });
      setTimeout(() => {
        if (!this.pending.has(id)) return;
        this.pending.delete(id);
        reject(new Error(`Compass LSP timeout: ${method}`));
      }, 5000);
    });
  }

  notify(method, params) {
    this.send({ jsonrpc: '2.0', method, params });
  }

  async syncDocument(document) {
    if (document.languageId !== LANGUAGE) return;
    await this.ensureStarted();
    this.notify('textDocument/didOpen', {
      textDocument: {
        uri: document.uri.toString(),
        languageId: LANGUAGE,
        version: document.version,
        text: document.getText(),
      },
    });
  }

  async syncUri(uri) {
    if (uri.scheme !== 'file') return;
    const document = await vscode.workspace.openTextDocument(uri);
    await this.syncDocument(document);
  }

  async indexWorkspaceDocuments() {
    await this.ensureStarted();
    const files = await vscode.workspace.findFiles(
      '**/*.zt',
      '**/{.git,node_modules,target,build,.release-install,.playwright-mcp}/**',
      5000,
    );
    for (const uri of files) {
      await this.syncUri(uri);
    }
  }
}

function documentSelector() {
  return [{ language: LANGUAGE, scheme: 'file' }, { language: LANGUAGE, scheme: 'untitled' }];
}

function resolveCliPath(context) {
  const configured = vscode.workspace.getConfiguration('zenith').get('cli.path');
  if (configured && configured.trim().length > 0) return configured;
  const repoRoot = path.resolve(context.extensionPath, '..', '..');
  const local = path.join(repoRoot, process.platform === 'win32' ? 'zt.exe' : 'zt');
  if (fs.existsSync(local)) return local;
  return process.platform === 'win32' ? 'zt.exe' : 'zt';
}

function projectTarget() {
  const editor = vscode.window.activeTextEditor;
  if (editor) {
    const folder = vscode.workspace.getWorkspaceFolder(editor.document.uri);
    if (folder) return folder.uri.fsPath;
    if (editor.document.uri.scheme === 'file') return editor.document.uri.fsPath;
  }
  const folder = vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0];
  return folder ? folder.uri.fsPath : '.';
}

function runTerminalCommand(context, command) {
  const cli = resolveCliPath(context);
  const terminal = vscode.window.createTerminal('Zenith');
  terminal.show(true);
  terminal.sendText(`"${cli}" ${command} "${projectTarget()}"`);
}

function activate(context) {
  const diagnostics = vscode.languages.createDiagnosticCollection('zenith');
  const client = new CompassClient(context, diagnostics);

  context.subscriptions.push(diagnostics);
  context.subscriptions.push({ dispose: () => client.stop() });

  context.subscriptions.push(vscode.workspace.onDidOpenTextDocument((doc) => client.syncDocument(doc)));
  context.subscriptions.push(vscode.workspace.onDidChangeTextDocument((event) => client.syncDocument(event.document)));
  context.subscriptions.push(vscode.workspace.onDidCloseTextDocument((doc) => {
    if (doc.languageId !== LANGUAGE) return;
    if (doc.uri.scheme !== 'file') {
      client.notify('textDocument/didClose', { textDocument: { uri: doc.uri.toString() } });
    }
    diagnostics.delete(doc.uri);
  }));

  const watcher = vscode.workspace.createFileSystemWatcher('**/*.zt');
  context.subscriptions.push(watcher);
  context.subscriptions.push(watcher.onDidCreate((uri) => client.syncUri(uri)));
  context.subscriptions.push(watcher.onDidChange((uri) => client.syncUri(uri)));
  context.subscriptions.push(watcher.onDidDelete((uri) => {
    client.notify('textDocument/didClose', { textDocument: { uri: uri.toString() } });
    diagnostics.delete(uri);
  }));

  for (const doc of vscode.workspace.textDocuments) client.syncDocument(doc);
  client.indexWorkspaceDocuments();

  context.subscriptions.push(vscode.languages.registerHoverProvider(documentSelector(), {
    async provideHover(document, position) {
      await client.syncDocument(document);
      const result = await client.request('textDocument/hover', {
        textDocument: { uri: document.uri.toString() },
        position,
      });
      if (!result || !result.contents) return undefined;
      const value = typeof result.contents === 'string' ? result.contents : result.contents.value;
      return new vscode.Hover(new vscode.MarkdownString(value || ''));
    },
  }));

  context.subscriptions.push(vscode.languages.registerDefinitionProvider(documentSelector(), {
    async provideDefinition(document, position) {
      await client.syncDocument(document);
      const result = await client.request('textDocument/definition', {
        textDocument: { uri: document.uri.toString() },
        position,
      });
      if (!result || !result.uri || !result.range) return undefined;
      return new vscode.Location(vscode.Uri.parse(result.uri), new vscode.Range(
        result.range.start.line,
        result.range.start.character,
        result.range.end.line,
        result.range.end.character,
      ));
    },
  }));

  context.subscriptions.push(vscode.languages.registerDocumentFormattingEditProvider(documentSelector(), {
    async provideDocumentFormattingEdits(document, options) {
      await client.syncDocument(document);
      const result = await client.request('textDocument/formatting', {
        textDocument: { uri: document.uri.toString() },
        options,
      });
      return (result || []).map((edit) => new vscode.TextEdit(
        new vscode.Range(
          edit.range.start.line,
          edit.range.start.character,
          edit.range.end.line,
          edit.range.end.character,
        ),
        edit.newText,
      ));
    },
  }));

  context.subscriptions.push(vscode.languages.registerCompletionItemProvider(documentSelector(), {
    async provideCompletionItems(document, position) {
      await client.syncDocument(document);
      const result = await client.request('textDocument/completion', {
        textDocument: { uri: document.uri.toString() },
        position,
      });
      return (result || []).map((item) => {
        const completion = new vscode.CompletionItem(
          item.label,
          item.kind ? item.kind - 1 : vscode.CompletionItemKind.Text,
        );
        if (item.detail) completion.detail = item.detail;
        if (item.documentation) completion.documentation = item.documentation;
        if (item.insertText) {
          completion.insertText = item.insertTextFormat === 2
            ? new vscode.SnippetString(item.insertText)
            : item.insertText;
        }
        return completion;
      });
    },
  }, '.', ':', '<'));

  context.subscriptions.push(vscode.commands.registerCommand('zenith.check', () => runTerminalCommand(context, 'check')));
  context.subscriptions.push(vscode.commands.registerCommand('zenith.build', () => runTerminalCommand(context, 'build')));
  context.subscriptions.push(vscode.commands.registerCommand('zenith.run', () => runTerminalCommand(context, 'run')));
}

function deactivate() {}

module.exports = { activate, deactivate };
