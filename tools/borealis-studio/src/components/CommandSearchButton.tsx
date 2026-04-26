import { AnimatePresence, motion } from "framer-motion";
import { Command, Search } from "lucide-react";
import React, { useEffect, useRef, useState } from "react";
import { ICON_STROKE } from "../constants";

export function CommandSearchButton() {
  const [searchOpen, setSearchOpen] = useState(false);
  const searchRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    function handlePointerDown(event: MouseEvent) {
      if (!searchRef.current?.contains(event.target as Node)) setSearchOpen(false);
    }

    function handleKeyDown(event: KeyboardEvent) {
      if (event.key === "Escape") setSearchOpen(false);
    }

    window.addEventListener("pointerdown", handlePointerDown);
    window.addEventListener("keydown", handleKeyDown);
    return () => {
      window.removeEventListener("pointerdown", handlePointerDown);
      window.removeEventListener("keydown", handleKeyDown);
    };
  }, []);

  return (
    <div
      className="search-popover"
      ref={searchRef}
      onKeyDown={(event) => {
        if (event.key === "Escape") setSearchOpen(false);
      }}
    >
      <button
        aria-expanded={searchOpen}
        aria-label="Open command search"
        className={`top-icon-button ${searchOpen ? "active" : ""}`}
        onClick={() => setSearchOpen((open) => !open)}
        title="Search"
      >
        <Search size={15} strokeWidth={ICON_STROKE} />
      </button>
      <AnimatePresence>
        {searchOpen ? (
          <>
            <button
              aria-label="Close command search"
              className="command-popover-backdrop"
              onPointerDown={() => setSearchOpen(false)}
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
                  onKeyDown={(event) => {
                    if (event.key === "Escape") setSearchOpen(false);
                  }}
                  placeholder="Search files, entities, commands"
                />
                <Command size={14} strokeWidth={ICON_STROKE} />
              </div>
              <div className="command-suggestions">
                <button>Open player_controller.zt</button>
                <button>Select Player entity</button>
                <button>Show project assets</button>
              </div>
            </motion.div>
          </>
        ) : null}
      </AnimatePresence>
    </div>
  );
}
