"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const path = require("path");
const vscode_1 = require("vscode");
const node_1 = require("vscode-languageclient/node");
let client;
function activate(context) {
    // The server is our native compiled LSP executable
    const serverPath = context.asAbsolutePath(path.join('..', '..', 'zt-lsp.exe') // Adjust path as needed, assuming we run from inside tooling/vscode for now
    );
    const run = {
        command: serverPath,
        args: []
    };
    const serverOptions = {
        run: run,
        debug: run
    };
    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'zenith' }],
        synchronize: {
            fileEvents: vscode_1.workspace.createFileSystemWatcher('**/.clientrc')
        }
    };
    client = new node_1.LanguageClient('zenithLanguageServer', 'Zenith Language Server', serverOptions, clientOptions);
    client.start();
}
function deactivate() {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
//# sourceMappingURL=extension.js.map