import { useState, useEffect } from "react";
import { Pencil, Search, Keyboard, Info, Command, Check, X } from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { AnimatePresence, motion } from "framer-motion";

export function KeymapModule() {
  const commands = useWorkspaceStore((s) => s.commands);
  const { settings, updateSettings } = useWorkspaceStore();
  const [editingCmdId, setEditingCmdId] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState("");

  const getShortcutForCommand = (id: string) => {
    for (const [combo, cmdId] of Object.entries(settings.keymap || {})) {
      if (cmdId === id) return combo;
    }
    return null;
  };

  const commandList = Object.values(commands)
    .filter(cmd => 
      cmd.title.toLowerCase().includes(searchQuery.toLowerCase()) || 
      (cmd.category || "System").toLowerCase().includes(searchQuery.toLowerCase()) ||
      cmd.id.toLowerCase().includes(searchQuery.toLowerCase())
    )
    .sort((a, b) => {
      const catA = a.category || "Z";
      const catB = b.category || "Z";
      if (catA < catB) return -1;
      if (catA > catB) return 1;
      return a.title.localeCompare(b.title);
    });

  const handleSave = (cmdId: string, newCombo: string) => {
    const newKeymap = { ...settings.keymap };

    if (newKeymap[newCombo]) {
      delete newKeymap[newCombo];
    }

    const oldCombo = getShortcutForCommand(cmdId);
    if (oldCombo && oldCombo !== newCombo) {
      delete newKeymap[oldCombo];
    }

    newKeymap[newCombo] = cmdId;
    updateSettings({ keymap: newKeymap });
    setEditingCmdId(null);
  };

  return (
    <div className="space-y-12 animate-in fade-in slide-in-from-bottom-4 duration-1000 h-full flex flex-col">
      {/* Sovereign Header */}
      <header className="space-y-8 shrink-0">
        <div className="flex flex-col lg:flex-row lg:items-end justify-between gap-8">
          <div className="space-y-4">
            <div className="flex items-center gap-3">
              <div className="h-2 w-8 bg-primary/30 rounded-full" />
              <div className="text-[10px] font-bold uppercase tracking-[0.4em] text-primary/60">Configuração de Fluxo</div>
            </div>
            <h2 className="text-[32px] font-bold tracking-tight text-ide-text">Mapeamento de Teclas</h2>
            <p className="text-[14px] leading-relaxed text-ide-text-dim/70 max-w-xl">
              Defina atalhos de alta performance para cada ação do sistema Taurus. 0ms de latência entre pensamento e execução.
            </p>
          </div>

          <div className="relative group w-full lg:w-[320px]">
            <Search size={14} className="pointer-events-none absolute left-4 top-1/2 -translate-y-1/2 text-ide-text-dim/40 transition-colors group-focus-within:text-primary" />
            <input 
              type="text"
              placeholder="Pesquisar comandos..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="w-full h-11 rounded-[20px] border border-ide-border bg-ide-panel/10 pl-11 pr-4 text-[13px] text-ide-text outline-none transition-all placeholder:text-ide-text-dim/30 focus:border-primary/40 focus:bg-ide-bg focus:ring-4 focus:ring-primary/5"
            />
          </div>
        </div>
      </header>

      {/* Command Registry */}
      <div className="flex-1 min-h-0 min-w-0 bg-ide-panel/5 border border-ide-border rounded-[32px] overflow-hidden flex flex-col">
        <div className="overflow-y-auto no-scrollbar divide-y divide-ide-border/40">
          {commandList.length > 0 ? (
            commandList.map((cmd) => {
              const isEditing = editingCmdId === cmd.id;
              const shortcut = getShortcutForCommand(cmd.id);

              return (
                <div
                  key={cmd.id}
                  className={`group relative flex items-center justify-between px-10 py-6 transition-all duration-500 ${
                    isEditing ? 'bg-primary/[0.03] ring-1 ring-inset ring-primary/10' : 'hover:bg-ide-panel/10'
                  }`}
                >
                  <div className="flex flex-col gap-1.5 min-w-0">
                    <div className="flex items-center gap-3">
                      <span className="text-[14px] font-bold text-ide-text truncate">
                        {cmd.title}
                      </span>
                      {cmd.category && (
                        <span className="shrink-0 px-2 py-0.5 rounded-full bg-ide-panel/20 text-[9px] font-bold uppercase tracking-widest text-ide-text-dim/50 border border-ide-border/50">
                          {cmd.category}
                        </span>
                      )}
                    </div>
                    <span className="text-[10px] text-ide-text-dim/40 font-mono tracking-tight uppercase">{cmd.id}</span>
                  </div>

                  <div className="flex items-center gap-8">
                    <AnimatePresence mode="wait">
                      {isEditing ? (
                        <motion.div
                          key="editing"
                          initial={{ opacity: 0, scale: 0.95 }}
                          animate={{ opacity: 1, scale: 1 }}
                          exit={{ opacity: 0, scale: 1.05 }}
                        >
                           <KeybindCapture
                             currentCombo={shortcut}
                             onSave={(combo) => handleSave(cmd.id, combo)}
                             onCancel={() => setEditingCmdId(null)}
                           />
                        </motion.div>
                      ) : (
                        <motion.div
                          key="static"
                          initial={{ opacity: 0 }}
                          animate={{ opacity: 1 }}
                          className="flex items-center gap-6"
                        >
                          <div className="flex gap-2">
                            {shortcut ? (
                              shortcut.split('+').map((key, i) => (
                                <kbd 
                                  key={i}
                                  className="inline-flex items-center justify-center min-w-[28px] h-7 px-2.5 rounded-lg border border-ide-border bg-ide-bg text-[10px] font-mono font-bold text-ide-text-dim shadow-xl shadow-black/5"
                                >
                                  {formatKey(key)}
                                </kbd>
                              ))
                            ) : (
                              <span className="text-[11px] text-ide-text-dim/20 italic tracking-wide">Desvinculado</span>
                            )}
                          </div>

                          <button
                            onClick={() => setEditingCmdId(cmd.id)}
                            className="h-9 w-9 flex items-center justify-center text-ide-text-dim/40 hover:bg-primary/10 hover:text-primary rounded-xl transition-all border border-transparent hover:border-primary/20"
                          >
                            <Pencil size={14} />
                          </button>
                        </motion.div>
                      )}
                    </AnimatePresence>
                  </div>
                </div>
              );
            })
          ) : (
            <div className="flex-1 flex flex-col items-center justify-center py-20 text-center opacity-40">
              <Search size={40} className="mb-4 text-ide-text-dim" />
              <p className="text-[13px] font-bold">Nenhum comando encontrado</p>
            </div>
          )}
        </div>
      </div>

      {/* Tip Card */}
      <footer className="p-8 rounded-[32px] border border-primary/20 bg-gradient-to-br from-primary/10 to-transparent flex items-start gap-6 group">
        <div className="h-10 w-10 shrink-0 flex items-center justify-center rounded-xl bg-primary text-white shadow-lg shadow-primary/20">
           <Command size={18} />
        </div>
        <div className="space-y-1.5 min-w-0">
          <div className="text-[12px] font-display font-bold text-primary italic">Zenith Engineering Tip</div>
          <p className="text-[12px] leading-relaxed text-ide-text-dim/80">
            Combine atalhos com o <b>Zenith Sanctuary</b> para criar um ambiente de fluxo ininterrupto. 
            Mapas mentais de comandos reduzem a carga cognitiva e aceleram o ciclo de feedback.
          </p>
        </div>
      </footer>
    </div>
  );
}

function KeybindCapture({
  currentCombo,
  onSave,
  onCancel,
}: {
  currentCombo: string | null;
  onSave: (combo: string) => void;
  onCancel: () => void;
}) {
  const [combo, setCombo] = useState<string>(currentCombo || "");

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      e.preventDefault();
      e.stopPropagation();

      if (e.key === "Escape") {
        onCancel();
        return;
      }

      if (e.key === "Enter") {
        if (combo) onSave(combo);
        else onCancel();
        return;
      }

      const keys = [];
      if (e.ctrlKey) keys.push("ctrl");
      if (e.shiftKey) keys.push("shift");
      if (e.altKey) keys.push("alt");
      if (e.metaKey) keys.push("meta");

      if (["Control", "Shift", "Alt", "Meta"].includes(e.key)) return;

      let keyName = e.key.toLowerCase();
      if (keyName === " ") keyName = "space";
      if (keyName === "arrowup") keyName = "up";
      if (keyName === "arrowdown") keyName = "down";
      if (keyName === "arrowleft") keyName = "left";
      if (keyName === "arrowright") keyName = "right";

      keys.push(keyName);
      setCombo(keys.join("+"));
    };

    window.addEventListener("keydown", handleKeyDown, { capture: true });
    return () => window.removeEventListener("keydown", handleKeyDown, { capture: true });
  }, [combo, onSave, onCancel]);

  return (
    <div className="flex items-center gap-6 bg-primary px-6 py-3 rounded-[20px] shadow-2xl shadow-primary/30 ring-4 ring-primary/10 transition-all">
      <div className="flex gap-2">
        {combo ? combo.split('+').map((k, i) => (
          <kbd key={i} className="bg-white/20 text-white min-w-[24px] h-6 px-1.5 rounded-lg flex items-center justify-center text-[10px] font-mono font-bold">
            {formatKey(k)}
          </kbd>
        )) : <span className="text-white/60 text-[10px] animate-pulse uppercase tracking-[0.2em] font-bold">Aguardando entrada...</span>}
      </div>
      <div className="h-4 w-px bg-white/20" />
      <div className="flex gap-4">
         <button onClick={() => combo && onSave(combo)} className="text-white hover:scale-110 transition-transform">
            <Check size={14} />
         </button>
         <button onClick={onCancel} className="text-white/60 hover:text-white hover:scale-110 transition-transform">
            <X size={14} />
         </button>
      </div>
    </div>
  );
}

function formatKey(key: string) {
  if (key === "ctrl") return "Ctrl";
  if (key === "shift") return "⇧";
  if (key === "alt") return "Alt";
  if (key === "meta") return "⌘";
  if (key === "up") return "↑";
  if (key === "down") return "↓";
  if (key === "left") return "←";
  if (key === "right") return "→";
  if (key === "space") return "Space";
  return key.length === 1 ? key.toUpperCase() : key;
}

