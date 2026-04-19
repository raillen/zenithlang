import * as path from 'path';
import { workspace, ExtensionContext } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    Executable
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
    // The server is our native compiled LSP executable
    const serverPath = context.asAbsolutePath(
        path.join('..', '..', 'zt-lsp.exe') // Adjust path as needed, assuming we run from inside tooling/vscode for now
    );

    const run: Executable = {
        command: serverPath,
        args: []
    };

    const serverOptions: ServerOptions = {
        run: run,
        debug: run
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'zenith' }],
        synchronize: {
            fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
        }
    };

    client = new LanguageClient(
        'zenithLanguageServer',
        'Zenith Language Server',
        serverOptions,
        clientOptions
    );

    client.start();
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
