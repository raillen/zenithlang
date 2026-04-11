const bufferCache = new Map<string, string>();
const pendingLoads = new Map<string, Promise<string>>();

function normalizePath(path: string) {
  return path.replace(/\\/g, "/").replace(/\/+$/g, "").toLowerCase();
}

export function getCachedBuffer(path: string) {
  return bufferCache.get(path);
}

export function rememberBuffer(path: string, content: string) {
  bufferCache.set(path, content);
}

export function forgetBuffer(path: string) {
  bufferCache.delete(path);
  pendingLoads.delete(path);
}

export function clearEditorBuffers() {
  bufferCache.clear();
  pendingLoads.clear();
}

export function remapEditorBuffer(oldPath: string, newPath: string) {
  const content = bufferCache.get(oldPath);
  if (content === undefined) return;

  bufferCache.set(newPath, content);
  bufferCache.delete(oldPath);
  pendingLoads.delete(oldPath);
}

export function remapEditorBuffersInTree(oldRootPath: string, newRootPath: string) {
  const normalizedRoot = normalizePath(oldRootPath);

  for (const key of Array.from(bufferCache.keys())) {
    const normalizedKey = normalizePath(key);
    if (
      normalizedKey === normalizedRoot ||
      normalizedKey.startsWith(`${normalizedRoot}/`)
    ) {
      const suffix = key.replace(/\\/g, "/").slice(oldRootPath.replace(/\\/g, "/").length);
      const nextKey = `${newRootPath.replace(/[\\/]+$/g, "")}${suffix}`;
      const content = bufferCache.get(key);
      if (content !== undefined) {
        bufferCache.set(nextKey, content);
      }
      bufferCache.delete(key);
      pendingLoads.delete(key);
    }
  }
}

export function removeEditorBuffersInTree(rootPath: string) {
  const normalizedRoot = normalizePath(rootPath);

  for (const key of Array.from(bufferCache.keys())) {
    const normalizedKey = normalizePath(key);
    if (
      normalizedKey === normalizedRoot ||
      normalizedKey.startsWith(`${normalizedRoot}/`)
    ) {
      bufferCache.delete(key);
    }
  }

  for (const key of Array.from(pendingLoads.keys())) {
    const normalizedKey = normalizePath(key);
    if (
      normalizedKey === normalizedRoot ||
      normalizedKey.startsWith(`${normalizedRoot}/`)
    ) {
      pendingLoads.delete(key);
    }
  }
}

export function getPendingBufferLoad(path: string) {
  return pendingLoads.get(path);
}

export function setPendingBufferLoad(path: string, promise: Promise<string>) {
  pendingLoads.set(path, promise);
}

export function clearPendingBufferLoad(path: string) {
  pendingLoads.delete(path);
}
