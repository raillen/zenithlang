import React, { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import CodeMirror from '@uiw/react-codemirror';
import { Check, Code, Copy, Lightning, LinkSimple, Repeat } from '@phosphor-icons/react';
import { getPlaygroundEmbed } from '../data/playgroundSamples';
import { zenithLanguage } from '../utils/zenithLanguage';
import { runZenithPlayground } from '../utils/zenithPlayground';

const requestFrame = () =>
  new Promise((resolve) => {
    window.requestAnimationFrame(() => resolve());
  });

const getStatusCopy = (status, result) => {
  if (status === 'running') {
    return 'Running';
  }

  if (status === 'error') {
    return 'Compile error';
  }

  if (result?.durationMs) {
    return `Ready in ${result.durationMs} ms`;
  }

  return 'Ready';
};

const getStatusClasses = (status) => {
  if (status === 'running') {
    return 'border-[#0F9F6E]/20 bg-[#0F9F6E]/8 text-[#0F9F6E]';
  }

  if (status === 'error') {
    return 'border-rose-500/20 bg-rose-500/8 text-rose-700';
  }

  return 'border-black/10 bg-black/[0.03] text-neutral/70';
};

const getPlaygroundUrlKeys = (embedId) => ({
  code: `zp-${embedId}-code`,
  preset: `zp-${embedId}-preset`,
});

const getResolvedPresetId = (embed, presetId, fallbackId) => {
  if (!presetId) {
    return fallbackId;
  }

  return embed.presets.some((preset) => preset.id === presetId) ? presetId : fallbackId;
};

const buildPlaygroundUrl = (embed, defaultPreset, activePresetId, code) => {
  if (typeof window === 'undefined') {
    return { href: '', path: '' };
  }

  const params = new URLSearchParams(window.location.search);
  const keys = getPlaygroundUrlKeys(embed.id);
  const resolvedPresetId = getResolvedPresetId(embed, activePresetId, defaultPreset.id);
  const presetCode = embed.presets.find((preset) => preset.id === resolvedPresetId)?.code ?? defaultPreset.code;

  if (resolvedPresetId !== defaultPreset.id) {
    params.set(keys.preset, resolvedPresetId);
  } else {
    params.delete(keys.preset);
  }

  if (code !== presetCode) {
    params.set(keys.code, code);
  } else {
    params.delete(keys.code);
  }

  const query = params.toString();
  const path = `${window.location.pathname}${query ? `?${query}` : ''}${window.location.hash}`;

  return {
    href: `${window.location.origin}${path}`,
    path,
  };
};

const readPlaygroundState = (embed, defaultPreset) => {
  if (typeof window === 'undefined') {
    return {
      presetId: defaultPreset.id,
      code: defaultPreset.code,
    };
  }

  const params = new URLSearchParams(window.location.search);
  const keys = getPlaygroundUrlKeys(embed.id);
  const presetId = getResolvedPresetId(embed, params.get(keys.preset), defaultPreset.id);
  const code = params.has(keys.code) ? params.get(keys.code) ?? '' : embed.presets.find((preset) => preset.id === presetId)?.code ?? defaultPreset.code;

  return { presetId, code };
};

const CodePlayground = ({ embedId = 'zenith-lab' }) => {
  const embed = useMemo(() => getPlaygroundEmbed(embedId) ?? getPlaygroundEmbed('zenith-lab'), [embedId]);
  const defaultPreset = embed.presets.find((preset) => preset.id === embed.defaultPresetId) ?? embed.presets[0];
  const initialUrlState = useMemo(() => readPlaygroundState(embed, defaultPreset), [embed, defaultPreset]);

  const editorViewRef = useRef(null);
  const [activePresetId, setActivePresetId] = useState(initialUrlState.presetId);
  const [code, setCode] = useState(initialUrlState.code);
  const [result, setResult] = useState({
    ok: true,
    transpiled: '',
    output: '',
    error: '',
    durationMs: 0,
  });
  const [status, setStatus] = useState('idle');
  const [copiedKey, setCopiedKey] = useState('');

  const runSource = useCallback(async (nextCode) => {
    setStatus('running');
    await requestFrame();

    const nextResult = await runZenithPlayground(nextCode);
    setResult(nextResult);
    setStatus(nextResult.ok ? 'success' : 'error');
  }, []);

  const syncFromUrl = useCallback(async () => {
    const nextState = readPlaygroundState(embed, defaultPreset);
    setActivePresetId(nextState.presetId);
    setCode(nextState.code);
    await runSource(nextState.code);
  }, [defaultPreset, embed, runSource]);

  useEffect(() => {
    syncFromUrl();
  }, [syncFromUrl]);

  useEffect(() => {
    const handlePopState = () => {
      syncFromUrl();
    };

    window.addEventListener('popstate', handlePopState);
    return () => window.removeEventListener('popstate', handlePopState);
  }, [syncFromUrl]);

  useEffect(() => {
    const timeoutId = window.setTimeout(() => {
      const nextUrl = buildPlaygroundUrl(embed, defaultPreset, activePresetId, code);
      window.history.replaceState(window.history.state, '', nextUrl.path);
    }, 280);

    return () => window.clearTimeout(timeoutId);
  }, [activePresetId, code, defaultPreset, embed]);

  const activePreset = embed.presets.find((preset) => preset.id === activePresetId) ?? defaultPreset;
  const isCustomCode = code !== activePreset.code;

  const handlePresetSelect = async (presetId) => {
    const selectedPreset = embed.presets.find((preset) => preset.id === presetId);

    if (!selectedPreset) {
      return;
    }

    setActivePresetId(selectedPreset.id);
    setCode(selectedPreset.code);
    await runSource(selectedPreset.code);
  };

  const handleReset = async () => {
    setCode(activePreset.code);
    await runSource(activePreset.code);
  };

  const flashCopyState = (nextKey) => {
    setCopiedKey(nextKey);
    window.setTimeout(() => {
      setCopiedKey((currentValue) => (currentValue === nextKey ? '' : currentValue));
    }, 1400);
  };

  const handleCopy = async (nextKey, text) => {
    if (!text || !navigator.clipboard) {
      return;
    }

    await navigator.clipboard.writeText(text);
    flashCopyState(nextKey);
  };

  const handleCopyLink = async () => {
    if (!navigator.clipboard) {
      return;
    }

    const nextUrl = buildPlaygroundUrl(embed, defaultPreset, activePresetId, code);
    await navigator.clipboard.writeText(nextUrl.href);
    flashCopyState('share');
  };

  const handleInsertSnippet = (snippet) => {
    if (!snippet?.template) {
      return;
    }

    const view = editorViewRef.current;

    if (!view) {
      setCode((currentCode) => {
        const separator = currentCode.trim() ? '\n\n' : '';
        return `${currentCode}${separator}${snippet.template}`;
      });
      return;
    }

    const selection = view.state.selection.main;
    const before = view.state.doc.sliceString(Math.max(0, selection.from - 1), selection.from);
    const after = view.state.doc.sliceString(selection.to, Math.min(view.state.doc.length, selection.to + 1));
    const leadingBreak = selection.from > 0 && before !== '\n' ? '\n\n' : '';
    const trailingBreak = selection.to < view.state.doc.length && after !== '\n' ? '\n\n' : '';
    const insertText = `${leadingBreak}${snippet.template}${trailingBreak}`;

    view.dispatch({
      changes: { from: selection.from, to: selection.to, insert: insertText },
      selection: { anchor: selection.from + insertText.length },
      scrollIntoView: true,
    });

    setCode(view.state.doc.toString());
    view.focus();
  };

  const outputText = result.ok
    ? result.output || 'No stdout yet. Add print(...) inside main() to see output here.'
    : result.error || 'The compiler could not execute this snippet.';

  return (
    <section
      data-z-id="code-playground"
      className="not-prose my-10 overflow-hidden rounded-lg border border-black/10 bg-white shadow-[0_20px_40px_-24px_rgba(15,23,42,0.18)]"
    >
      <div className="border-b border-black/5 bg-[#FBFCFD] px-4 py-4 sm:px-6">
        <div className="flex flex-col gap-4 lg:flex-row lg:items-start lg:justify-between">
          <div className="max-w-2xl">
            <div className="flex items-center gap-2 text-[11px] font-bold uppercase tracking-[0.18em] text-neutral/45">
              <span className="inline-flex h-2 w-2 rounded-full bg-[#0F9F6E]" aria-hidden="true" />
              Browser runtime
            </div>

            <h2 className="mt-3 text-2xl font-display tracking-tight text-neutral sm:text-[2rem]">
              {embed.title}
            </h2>

            <p className="mt-2 max-w-[62ch] text-sm leading-relaxed text-neutral/70">
              {embed.subtitle}
            </p>
          </div>

          <div
            className={`inline-flex items-center gap-2 rounded-md border px-3 py-2 text-[11px] font-bold uppercase tracking-[0.18em] ${getStatusClasses(
              status
            )}`}
          >
            <span className="inline-flex h-2 w-2 rounded-full bg-current opacity-70" aria-hidden="true" />
            {getStatusCopy(status, result)}
          </div>
        </div>

        <div className="mt-4 flex flex-col gap-3 xl:flex-row xl:items-center xl:justify-between">
          <div className="flex flex-wrap gap-2">
            {embed.presets.map((preset) => {
              const isActive = preset.id === activePresetId;

              return (
                <button
                  key={preset.id}
                  type="button"
                  onClick={() => handlePresetSelect(preset.id)}
                  className={`rounded-md border px-3 py-2 text-left transition-all active:scale-[0.99] ${
                    isActive
                      ? 'border-primary/20 bg-primary/10 text-primary'
                      : 'border-black/8 bg-white text-neutral/70 hover:border-primary/20 hover:text-neutral'
                  }`}
                >
                  <div className="text-[11px] font-bold uppercase tracking-[0.18em]">{preset.label}</div>
                  <div className="mt-1 text-xs text-current/70">{preset.summary}</div>
                </button>
              );
            })}

            {isCustomCode ? (
              <div className="inline-flex items-center rounded-md border border-dashed border-black/10 px-3 py-2 text-[11px] font-bold uppercase tracking-[0.18em] text-neutral/45">
                Custom
              </div>
            ) : null}
          </div>

          <div className="flex flex-wrap gap-2">
            <button
              type="button"
              onClick={() => runSource(code)}
              disabled={status === 'running'}
              className="inline-flex items-center gap-2 rounded-md bg-[#111111] px-4 py-2 text-sm font-semibold text-white transition-all hover:-translate-y-[1px] disabled:cursor-not-allowed disabled:opacity-60"
            >
              <Lightning size={16} weight="fill" />
              Run
            </button>

            <button
              type="button"
              onClick={handleReset}
              className="inline-flex items-center gap-2 rounded-md border border-black/8 bg-white px-4 py-2 text-sm font-semibold text-neutral/75 transition-all hover:-translate-y-[1px] hover:border-primary/20 hover:text-neutral"
            >
              <Repeat size={16} />
              Reset
            </button>

            <button
              type="button"
              onClick={handleCopyLink}
              className="inline-flex items-center gap-2 rounded-md border border-black/8 bg-white px-4 py-2 text-sm font-semibold text-neutral/75 transition-all hover:-translate-y-[1px] hover:border-primary/20 hover:text-neutral"
            >
              {copiedKey === 'share' ? <Check size={16} /> : <LinkSimple size={16} />}
              Link
            </button>
          </div>
        </div>

        <div className="mt-3 flex flex-wrap items-center gap-x-4 gap-y-1 text-xs leading-relaxed text-neutral/55">
          <span>{embed.hint}</span>
          <span className="hidden h-1 w-1 rounded-full bg-black/15 sm:inline-flex" aria-hidden="true" />
          <span>{embed.shareHint}</span>
        </div>
      </div>

      <div className="grid xl:grid-cols-[minmax(0,1.2fr)_minmax(320px,0.95fr)]">
        <section className="border-b border-black/5 xl:border-b-0 xl:border-r xl:border-black/5">
          <div className="flex flex-wrap items-center justify-between gap-3 border-b border-black/5 px-4 py-3 sm:px-6">
            <div className="flex items-center gap-2 text-sm font-semibold text-neutral">
              <Code size={16} />
              Source
            </div>

            <div className="flex items-center gap-3">
              <span className="text-[11px] font-bold uppercase tracking-[0.18em] text-neutral/40">
                main() auto-run
              </span>
            </div>
          </div>

          <div className="border-b border-black/5 bg-[#FCFCFD] px-4 py-3 sm:px-6">
            <div className="flex flex-wrap items-start gap-2">
              <span className="pt-2 text-[11px] font-bold uppercase tracking-[0.18em] text-neutral/40">
                Insercoes rápidas
              </span>

              {embed.inserts.map((snippet) => (
                <button
                  key={snippet.id}
                  type="button"
                  onClick={() => handleInsertSnippet(snippet)}
                  className="inline-flex items-center gap-2 rounded-md border border-black/8 bg-white px-3 py-2 text-left text-xs text-neutral/70 transition-all hover:-translate-y-[1px] hover:border-primary/20 hover:text-neutral"
                >
                  <span className="font-semibold text-neutral">{snippet.label}</span>
                  <span className="text-neutral/45">{snippet.summary}</span>
                </button>
              ))}
            </div>
          </div>

          <div className="min-h-[420px] bg-white">
            <CodeMirror
              value={code}
              height="420px"
              extensions={[zenithLanguage]}
              theme="light"
              basicSetup={{
                foldGutter: false,
                dropCursor: false,
                allowMultipleSelections: false,
                highlightActiveLineGutter: true,
              }}
              className="zenith-playground-editor"
              onCreateEditor={(view) => {
                editorViewRef.current = view;
              }}
              onChange={(value) => setCode(value)}
            />
          </div>
        </section>

        <section className="grid min-h-[420px] grid-rows-[minmax(0,1fr)_220px] bg-[#FCFCFD]">
          <div className="min-h-0">
            <div className="flex items-center justify-between border-b border-black/5 px-4 py-3 sm:px-6">
              <div className="flex items-center gap-2 text-sm font-semibold text-neutral">
                <Code size={16} />
                Lua output
              </div>

              <button
                type="button"
                onClick={() => handleCopy('lua', result.transpiled)}
                className="inline-flex items-center gap-2 rounded-md border border-black/8 bg-white px-3 py-1.5 text-[11px] font-bold uppercase tracking-[0.16em] text-neutral/60 transition-all hover:border-primary/20 hover:text-primary"
              >
                {copiedKey === 'lua' ? <Check size={14} /> : <Copy size={14} />}
                Copy
              </button>
            </div>

            <div className="zenith-playground-panel h-full overflow-auto px-4 py-4 sm:px-6">
              <pre className="m-0 min-h-full overflow-auto rounded-md bg-[#F6F8FA] p-4 text-[13px] leading-6 text-neutral/80">
                <code>{result.transpiled || '-- Run the playground to inspect generated Lua.'}</code>
              </pre>
            </div>
          </div>

          <div className="min-h-0 border-t border-black/5">
            <div className="flex items-center justify-between border-b border-black/5 px-4 py-3 sm:px-6">
              <div className="flex items-center gap-2 text-sm font-semibold text-neutral">
                <Lightning size={16} weight="fill" />
                Output
              </div>

              <button
                type="button"
                onClick={() => handleCopy('output', outputText)}
                className="inline-flex items-center gap-2 rounded-md border border-black/8 bg-white px-3 py-1.5 text-[11px] font-bold uppercase tracking-[0.16em] text-neutral/60 transition-all hover:border-primary/20 hover:text-primary"
              >
                {copiedKey === 'output' ? <Check size={14} /> : <Copy size={14} />}
                Copy
              </button>
            </div>

            <div className="zenith-playground-panel h-full overflow-auto px-4 py-4 sm:px-6">
              <pre
                className={`m-0 h-full overflow-auto rounded-md p-4 text-[13px] leading-6 ${
                  result.ok
                    ? 'bg-[#111111] text-white'
                    : 'bg-rose-50 text-rose-700 ring-1 ring-rose-500/15'
                }`}
              >
                <code>{outputText}</code>
              </pre>
            </div>
          </div>
        </section>
      </div>
    </section>
  );
};

export default CodePlayground;
