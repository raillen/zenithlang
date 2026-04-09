type DebugMetaValue = string | number | boolean | null | undefined;

function toKebabCase(value: string) {
  return value
    .replace(/([a-z0-9])([A-Z])/g, '$1-$2')
    .replace(/[^a-zA-Z0-9]+/g, '-')
    .replace(/^-+|-+$/g, '')
    .toLowerCase();
}

// Important UI surfaces should expose stable `data-ui-*` hooks so they can
// be found quickly in DevTools and reused by future debugging flows.
export function uiMarker(
  name: string,
  meta: Record<string, DebugMetaValue> = {}
): Record<`data-${string}`, string> {
  const attributes: Record<`data-${string}`, string> = {
    'data-ui': name,
  };

  Object.entries(meta).forEach(([key, value]) => {
    if (value === null || value === undefined) {
      return;
    }

    attributes[`data-ui-${toKebabCase(key)}`] = String(value);
  });

  return attributes;
}
