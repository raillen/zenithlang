import { AnimatePresence, motion } from "framer-motion";
import { Box, Command, FileCode2, Package, Search, Terminal } from "lucide-react";
import React, { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { ICON_STROKE } from "../constants";
import type { ProjectAsset, SceneEntity, ScriptDocument } from "../types";
import { searchAll, SearchResult, STUDIO_COMMANDS } from "../utils/fuzzySearch";

const CATEGORY_ICONS: Record<string, React.ReactNode> = {
  entity: <Box size={13} strokeWidth={ICON_STROKE} />,
  asset: <Package size={13} strokeWidth={ICON_STROKE} />,
  script: <FileCode2 size={13} strokeWidth={ICON_STROKE} />,
  command: <Terminal size={13} strokeWidth={ICON_STROKE} />,
};

export function CommandSearchButton({
  entities = [],
  assets = [],
  scripts = [],
  onSelectEntity,
  onOpenScript,
  onExecuteCommand,
}: {
  entities?: SceneEntity[];
  assets?: ProjectAsset[];
  scripts?: ScriptDocument[];
  onSelectEntity?: (id: string) => void;
  onOpenScript?: (path: string) => void;
  onExecuteCommand?: (id: string) => void;
}) {
  const [searchOpen, setSearchOpen] = useState(false);
  const [query, setQuery] = useState("");
  const [selectedIndex, setSelectedIndex] = useState(0);
  const searchRef = useRef<HTMLDivElement | null>(null);
  const inputRef = useRef<HTMLInputElement | null>(null);

  const results = useMemo(
    () => searchAll(query, entities, assets, scripts, STUDIO_COMMANDS),
    [query, entities, assets, scripts],
  );

  const open = useCallback(() => {
    setSearchOpen(true);
    setQuery("");
    setSelectedIndex(0);
  }, []);

  const close = useCallback(() => {
    setSearchOpen(false);
    setQuery("");
  }, []);

  function activate(result: SearchResult) {
    close();
    if (result.category === "entity" && onSelectEntity) {
      onSelectEntity(result.id.replace("entity:", ""));
    } else if (result.category === "script" && onOpenScript) {
      onOpenScript(result.id.replace("script:", ""));
    } else if (result.category === "command" && onExecuteCommand) {
      onExecuteCommand(result.id.replace("command:", ""));
    }
  }

  useEffect(() => {
    function handleGlobalKey(event: KeyboardEvent) {
      if ((event.ctrlKey || event.metaKey) && event.shiftKey && event.key.toLowerCase() === "p") {
        event.preventDefault();
        searchOpen ? close() : open();
      }
    }

    function handlePointerDown(event: MouseEvent) {
      if (!searchRef.current?.contains(event.target as Node)) close();
    }

    window.addEventListener("keydown", handleGlobalKey);
    window.addEventListener("pointerdown", handlePointerDown);
    return () => {
      window.removeEventListener("keydown", handleGlobalKey);
      window.removeEventListener("pointerdown", handlePointerDown);
    };
  }, [searchOpen, close, open]);

  useEffect(() => {
    setSelectedIndex(0);
  }, [query]);

  function handleKeyDown(event: React.KeyboardEvent) {
    if (event.key === "Escape") { close(); return; }
    if (event.key === "ArrowDown") {
      event.preventDefault();
      setSelectedIndex((i) => Math.min(i + 1, results.length - 1));
    }
    if (event.key === "ArrowUp") {
      event.preventDefault();
      setSelectedIndex((i) => Math.max(i - 1, 0));
    }
    if (event.key === "Enter" && results[selectedIndex]) {
      event.preventDefault();
      activate(results[selectedIndex]);
    }
  }

  return (
    <div className="search-popover" ref={searchRef}>
      <button
        aria-expanded={searchOpen}
        aria-label="Open command search"
        className={`top-icon-button ${searchOpen ? "active" : ""}`}
        onClick={() => (searchOpen ? close() : open())}
        title="Search (Ctrl+Shift+P)"
      >
        <Search size={15} strokeWidth={ICON_STROKE} />
      </button>
      <AnimatePresence>
        {searchOpen ? (
          <>
            <button
              aria-label="Close command search"
              className="command-popover-backdrop"
              onPointerDown={close}
              tabIndex={-1}
              type="button"
            />
            <motion.div
              animate={{ opacity: 1, scale: 1, y: 0 }}
              className="command-popover"
              exit={{ opacity: 0, scale: 0.985, y: -4 }}
              initial={{ opacity: 0, scale: 0.985, y: -4 }}
              role="dialog"
              aria-label="Command search"
              transition={{ duration: 0.14, ease: [0.16, 1, 0.3, 1] }}
            >
              <div className="command-input">
                <Search size={15} strokeWidth={ICON_STROKE} />
                <input
                  autoFocus
                  ref={inputRef}
                  onKeyDown={handleKeyDown}
                  onChange={(e) => setQuery(e.target.value)}
                  placeholder="Search entities, assets, commands…"
                  value={query}
                />
                <kbd>Ctrl+Shift+P</kbd>
              </div>
              <div className="command-suggestions" role="listbox">
                {results.length === 0 ? (
                  <div className="command-empty">No results found</div>
                ) : (
                  results.map((result, index) => (
                    <button
                      key={result.id}
                      className={`command-result ${index === selectedIndex ? "selected" : ""}`}
                      onClick={() => activate(result)}
                      onPointerEnter={() => setSelectedIndex(index)}
                      role="option"
                      aria-selected={index === selectedIndex}
                    >
                      <span className="command-result-icon">{CATEGORY_ICONS[result.category]}</span>
                      <span className="command-result-label">{result.label}</span>
                      <span className="command-result-detail">{result.detail}</span>
                    </button>
                  ))
                )}
              </div>
            </motion.div>
          </>
        ) : null}
      </AnimatePresence>
    </div>
  );
}
