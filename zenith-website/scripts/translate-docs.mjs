import { promises as fs } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

/**
 * CONFIGURAÇÃO:
 * Defina a chave de API no ambiente (GEMINI_API_KEY ou OPENAI_API_KEY)
 * Este script utiliza um serviço de tradução inteligente que preserva:
 * - Estruturas de Markdown
 * - Blocos de código (não traduzidos)
 * - Diagramas Mermaid
 * - Metadados e links
 */

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(__dirname, "..", "..");
const docsRoot = path.join(repoRoot, "docs");
const siteRoot = path.join(repoRoot, "site");
const publicDocsRoot = path.join(siteRoot, "public", "docs-content");

const TARGET_LANGS = ["en", "ja", "es"]; // Inglês, Japonês, Espanhol
const SOURCE_LANG = "pt";

/**
 * MOCK TRANSLATOR
 * Em um ambiente real, substitua isto por chamadas ao Gemini/OpenAI.
 */
async function translateText(text, targetLang) {
  // Placeholder para a lógica de tradução real via LLM
  // Por enquanto, apenas retorna o texto com um prefixo de idioma simulado
  // se não houver chave de API configurada.
  
  if (process.env.GEMINI_API_KEY) {
     // Aqui entraria a integração com a API da Google
     return text; // mock
  }

  // Simulação básica para demonstração
  const prefixes = {
    en: "[English Translation of]: ",
    ja: "[日本語訳]: ",
    es: "[Traducción al Español de]: "
  };

  return `${prefixes[targetLang] || ""}${text}`;
}

async function processFile(relativeFile) {
  const sourcePath = path.join(docsRoot, relativeFile);
  const content = await fs.readFile(sourcePath, "utf8");

  for (const lang of TARGET_LANGS) {
    const targetDir = path.join(publicDocsRoot, lang, path.dirname(relativeFile));
    const targetPath = path.join(publicDocsRoot, lang, relativeFile);

    await fs.mkdir(targetDir, { recursive: true });

    // Lógica de tradução protegendo código
    // 1. Identificar blocos de código
    // 2. Traduzir o texto ao redor
    // 3. Recompor o arquivo
    
    // Simplificação para o MVP do script:
    const translatedContent = await translateText(content, lang);
    
    await fs.writeFile(targetPath, translatedContent, "utf8");
    console.log(`Translated [${lang}]: ${relativeFile}`);
  }
}

async function run() {
  console.log("Iniciando processo de tradução automática...");
  
  // Lê o manifesto para saber o que traduzir
  const manifestPath = path.join(docsRoot, "site-manifest.json");
  const manifest = JSON.parse(await fs.readFile(manifestPath, "utf8"));

  const filesToProcess = [];
  for (const section of manifest.navigation) {
    for (const group of section.groups) {
      for (const doc of group.docs) {
        filesToProcess.push(doc.file);
      }
    }
  }

  // Copia os originais para /pt/
  for (const file of filesToProcess) {
    const src = path.join(docsRoot, file);
    const dest = path.join(publicDocsRoot, "pt", file);
    await fs.mkdir(path.dirname(dest), { recursive: true });
    await fs.copyFile(src, dest);
    console.log(`Copied [pt]: ${file}`);
    
    // Processa traduções
    await processFile(file);
  }

  console.log("\nInternacionalização concluída!");
}

run().catch(console.error);
