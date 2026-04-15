import { promises as fs } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const siteRoot = path.resolve(__dirname, "..");
const repoRoot = path.resolve(siteRoot, "..");
const docsRoot = path.join(repoRoot, "docs");
const manifestPath = path.join(docsRoot, "site-manifest.json");
const publicDocsRoot = path.join(siteRoot, "public", "docs-content");
const publicManifestPath = path.join(siteRoot, "public", "docs-manifest.json");
const generatedModulePath = path.join(siteRoot, "src", "data", "docs.generated.js");
const generatedContentModulePath = path.join(siteRoot, "src", "data", "docs.content.generated.js");

try {
  await fs.access(docsRoot);
} catch (err) {
  console.warn(`[warning] Docs directory not found at ${docsRoot}. Skipping synchronization. Using existing generated data.`);
  process.exit(0);
}

const LANGS = ["pt", "en", "ja", "es"];
const manifest = JSON.parse(await fs.readFile(manifestPath, "utf8"));

// Ensure public directories exist
for (const lang of LANGS) {
  await fs.mkdir(path.join(publicDocsRoot, lang), { recursive: true });
}

const contentEntries = { pt: {}, en: {}, ja: {}, es: {} };
let totalFiles = 0;

for (const lang of LANGS) {
  console.log(`Processing language: ${lang}`);
  const currentLangRoot = lang === 'pt' ? docsRoot : path.join(docsRoot, lang);

  for (const section of manifest.navigation) {
    for (const group of section.groups) {
      for (const doc of group.docs) {
        const relativeFile = doc.file.replace(/\\/g, "/");
        const sourcePath = path.join(currentLangRoot, ...relativeFile.split("/"));
        
        try {
          await fs.access(sourcePath);
          const content = await fs.readFile(sourcePath, "utf8");
          contentEntries[lang][relativeFile] = content;
          
          // Se for Português, copia para a pasta public/pt como fallback
          if (lang === 'pt') {
            const targetPath = path.join(publicDocsRoot, "pt", ...relativeFile.split("/"));
            await fs.mkdir(path.dirname(targetPath), { recursive: true });
            await fs.copyFile(sourcePath, targetPath);
          }
          
          totalFiles++;
        } catch (err) {
          console.warn(`[${lang}] Missing file: ${relativeFile}`);
          // Fallback para Português se o arquivo traduzido não existir
          if (lang !== 'pt') {
             contentEntries[lang][relativeFile] = contentEntries['pt'][relativeFile] || "";
          }
        }
      }
    }
  }
}

await fs.writeFile(publicManifestPath, JSON.stringify(manifest, null, 2) + "\n", "utf8");

const generatedModule = `export const DOCS_MANIFEST = ${JSON.stringify(manifest, null, 2)};\n`;
await fs.writeFile(generatedModulePath, generatedModule, "utf8");

const generatedContentModule = `export const DOCS_CONTENT = ${JSON.stringify(contentEntries, null, 2)};\n`;
await fs.writeFile(generatedContentModulePath, generatedContentModule, "utf8");

console.log(`Synced and bundled ${totalFiles} documentation variants into site/src/data.`);
