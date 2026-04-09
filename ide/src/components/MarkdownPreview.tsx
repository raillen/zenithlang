import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import { useEffect, useState } from 'react';
import { safeInvoke as invoke } from '../utils/tauri';

interface MarkdownPreviewProps {
  path: string;
}

export function MarkdownPreview({ path }: MarkdownPreviewProps) {
  const [content, setContent] = useState('');

  useEffect(() => {
    const load = async () => {
      try {
        const text = await invoke<string>('read_file', { path });
        setContent(text);
      } catch (err) {
        console.error(err);
      }
    };
    load();
  }, [path]);

  return (
    <div className="flex-1 overflow-y-auto p-8 bg-white text-black prose prose-sm max-w-none prose-slate">
      <ReactMarkdown remarkPlugins={[remarkGfm]}>
        {content}
      </ReactMarkdown>
    </div>
  );
}
