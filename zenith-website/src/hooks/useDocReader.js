import { useState, useCallback } from 'react';
import { marked } from 'marked';
import { DOCS_CONTENT } from '../data/docs.content.generated';
import { UI_STRINGS } from '../data/translations';

const contentCache = new Map();

/**
 * Hook to manage Zenith documentation loading and parsing
 */
export const useDocReader = (lang) => {
  const [content, setContent] = useState('');
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const loadDoc = useCallback(async (file) => {
    if (!file) {
      setContent(`<div class="p-8 text-neutral/40 font-medium">${UI_STRINGS[lang]?.docViewer?.development || 'Section for development...'}</div>`);
      setLoading(false);
      return;
    }

    const cacheKey = `${lang}:${file}`;
    if (contentCache.has(cacheKey)) {
      setContent(contentCache.get(cacheKey));
      setLoading(false);
      return;
    }

    setLoading(true);
    setError(null);

    try {
      const langContent = DOCS_CONTENT[lang] || {};
      const text = langContent[file];

      if (typeof text !== 'string') {
        throw new Error(`Document [${lang}] not found: ${file}`);
      }
      
      // Remove Frontmatter
      const cleanMd = text.replace(/^---[\s\S]*?---/, '');
      
      // Parse Markdown
      const htmlContent = marked.parse(cleanMd);
      
      contentCache.set(cacheKey, htmlContent);
      setContent(htmlContent);
    } catch (err) {
      console.error("Failed to load document:", err);
      setError(err.message);
      setContent(`<div class="p-8 text-red-500 font-medium">${UI_STRINGS[lang]?.docViewer?.error || 'Failed to load document.'}</div>`);
    } finally {
      setLoading(false);
    }
  }, [lang]);

  return { content, loading, error, loadDoc };
};
