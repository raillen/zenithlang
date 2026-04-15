import { useState } from "react";
import { Type, Code, Layout, MousePointer, Search } from "lucide-react";
import { useWorkspaceStore } from "../../store/useWorkspaceStore";
import { ToggleSwitch } from "../ui/ToggleSwitch";

type EditorCategory = "all" | "typography" | "behavior" | "visuals";

export function EditorModule() {
  const { settings, updateSettings } = useWorkspaceStore();
  const [activeCategory, setActiveCategory] = useState<EditorCategory>("all");
  const [searchQuery, setSearchQuery] = useState("");

  const categories = [
    { id: "all", label: "Geral", icon: <Layout size={14} /> },
    { id: "typography", label: "Tipografia", icon: <Type size={14} /> },
    { id: "behavior", label: "Comportamento", icon: <Code size={14} /> },
    { id: "visuals", label: "Ajudas Visuais", icon: <MousePointer size={14} /> },
  ];

  const filteredSettings = {
    typography: [
      { id: "fontFamily", label: "Família da Fonte", description: "Define a tipografia principal do editor.", value: settings.editorFontFamily },
      { id: "fontSize", label: "Tamanho (PX)", description: "Escala visual do buffer de texto.", value: settings.editorFontSize },
      { id: "ligatures", label: "Ligaduras", description: "Ativa glifos combinados em fontes compatíveis.", value: settings.editorFontLigatures },
    ].filter(s => 
      s.label.toLowerCase().includes(searchQuery.toLowerCase()) || 
      s.description.toLowerCase().includes(searchQuery.toLowerCase())
    ),
    behavior: [
      { id: "formatOnSave", label: "Formatar ao Salvar", description: "Aciona o Zenith Formatter na persistência.", value: settings.editorFormatOnSave },
      { id: "wordWrap", label: "Quebra de Linha", description: "Ajusta o texto à largura da viewport.", value: settings.editorWordWrap },
      { id: "cursor", label: "Estilo do Cursor", description: "Animação visual do ponto de inserção.", value: settings.editorCursorBlinking },
    ].filter(s => 
      s.label.toLowerCase().includes(searchQuery.toLowerCase()) || 
      s.description.toLowerCase().includes(searchQuery.toLowerCase())
    ),
    visuals: [
      { id: "minimap", label: "Exibir Minimapa", description: "Renderiza uma visão estrutural miniatura.", value: settings.editorMinimap },
    ].filter(s => 
      s.label.toLowerCase().includes(searchQuery.toLowerCase()) || 
      s.description.toLowerCase().includes(searchQuery.toLowerCase())
    ),
  };

  const hasTypography = (activeCategory === "all" || activeCategory === "typography") && filteredSettings.typography.length > 0;
  const hasBehavior = (activeCategory === "all" || activeCategory === "behavior") && filteredSettings.behavior.length > 0;
  const hasVisuals = (activeCategory === "all" || activeCategory === "visuals") && filteredSettings.visuals.length > 0;

  return (
    <div className="space-y-10 animate-in fade-in slide-in-from-bottom-4 duration-1000 h-full flex flex-col">
      {/* Sovereign Header */}
      <header className="space-y-6 shrink-0">
        <div className="flex items-center justify-between">
          <div className="space-y-1">
            <div className="flex items-center gap-3">
              <div className="h-1.5 w-6 bg-primary/30 rounded-full" />
              <div className="text-[9px] font-bold uppercase tracking-[0.3em] text-primary/60">Buffer Engine</div>
            </div>
            <h2 className="text-[28px] font-bold tracking-tight text-ide-text">Configurações do Editor</h2>
          </div>
          
          <div className="hidden sm:flex items-center gap-2 px-3 py-1.5 rounded-lg border border-ide-border bg-ide-panel/20 text-[10px] font-bold text-ide-text-dim">
             <div className="h-1 w-1 rounded-full bg-emerald-500 animate-pulse" />
             <span>Monaco Core v0.45</span>
          </div>
        </div>

        {/* Search & Filter Bar */}
        <div className="flex flex-col sm:flex-row gap-3">
          <div className="relative flex-1 group">
            <Search size={14} className="absolute left-4 top-1/2 -translate-y-1/2 text-ide-text-dim/40 group-focus-within:text-primary transition-colors" />
            <input 
              type="text"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              placeholder="Pesquisar configurações (ex: font, wrap, ligatures)..."
              className="w-full bg-ide-panel/10 border border-ide-border rounded-xl pl-11 pr-4 py-2.5 text-[12px] text-ide-text focus:outline-none focus:border-primary/40 focus:ring-4 focus:ring-primary/5 transition-all"
            />
          </div>
          <div className="flex items-center gap-1 p-1 rounded-xl border border-ide-border bg-ide-panel/5">
             {categories.map((cat) => (
                <button
                  key={cat.id}
                  onClick={() => setActiveCategory(cat.id as EditorCategory)}
                  className={`flex items-center gap-2 px-3 py-1.5 rounded-lg text-[9px] font-bold uppercase tracking-wider transition-all ${
                    activeCategory === cat.id 
                      ? "bg-ide-bg text-primary shadow-sm border border-ide-border" 
                      : "text-ide-text-dim/60 hover:text-ide-text"
                  }`}
                >
                  {cat.icon}
                  <span className="hidden lg:inline">{cat.label}</span>
                </button>
             ))}
          </div>
        </div>
      </header>

      <div className="flex-1 overflow-y-auto no-scrollbar space-y-12 pb-20">
        {/* Typography Section */}
        {hasTypography && (
          <section className="space-y-6">
            <div className="flex items-center gap-4 px-2">
              <div className="h-px flex-1 bg-gradient-to-r from-transparent via-ide-border to-transparent" />
              <div className="text-[10px] font-bold uppercase tracking-[0.2em] text-ide-text-dim/40 italic">Tipografia & Fonte</div>
              <div className="h-px flex-1 bg-gradient-to-r from-transparent via-ide-border to-transparent" />
            </div>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              {filteredSettings.typography.some(s => s.id === "fontFamily") && (
                <div className="group space-y-2.5 p-5 rounded-2xl border border-ide-border bg-ide-panel/5 hover:bg-ide-panel/10 transition-all">
                  <label className="text-[10px] font-bold text-ide-text/40 uppercase tracking-[0.2em] ml-1">Família da Fonte</label>
                  <input 
                    type="text"
                    value={settings.editorFontFamily}
                    onChange={(e) => updateSettings({ editorFontFamily: e.target.value })}
                    className="w-full px-4 py-2.5 rounded-xl border border-ide-border bg-ide-bg/50 text-[13px] font-mono text-ide-text transition-all focus:outline-none focus:border-primary/40 focus:ring-4 focus:ring-primary/5"
                  />
                </div>
              )}
              {filteredSettings.typography.some(s => s.id === "fontSize") && (
                <div className="group space-y-2.5 p-5 rounded-2xl border border-ide-border bg-ide-panel/5 hover:bg-ide-panel/10 transition-all">
                  <label className="text-[10px] font-bold text-ide-text/40 uppercase tracking-[0.2em] ml-1">Tamanho (PX)</label>
                  <div className="flex items-center gap-4">
                    <input 
                      type="range"
                      min="8"
                      max="32"
                      value={settings.editorFontSize}
                      onChange={(e) => updateSettings({ editorFontSize: parseInt(e.target.value) })}
                      className="flex-1 h-1 bg-ide-border rounded-full accent-primary"
                    />
                    <span className="text-[14px] font-mono font-bold text-primary w-8">{settings.editorFontSize}</span>
                  </div>
                </div>
              )}
            </div>

            <div className="bg-ide-panel/5 border border-ide-border rounded-2xl overflow-hidden shadow-sm">
              {filteredSettings.typography.some(s => s.id === "ligatures") && (
                <SettingRow 
                  label="Ligaduras de Fonte" 
                  description="Ativa combinações especiais de glifos (ex: =>, !=) em famílias compatíveis."
                >
                  <ToggleSwitch 
                    checked={settings.editorFontLigatures} 
                    onChange={(val) => updateSettings({ editorFontLigatures: val })} 
                  />
                </SettingRow>
              )}
            </div>
          </section>
        )}

        {/* Behavior Section */}
        {hasBehavior && (
          <section className="space-y-6">
            <div className="flex items-center gap-4 px-2">
              <div className="h-px flex-1 bg-gradient-to-r from-transparent via-ide-border to-transparent" />
              <div className="text-[10px] font-bold uppercase tracking-[0.2em] text-ide-text-dim/40 italic">Comportamento do Buffer</div>
              <div className="h-px flex-1 bg-gradient-to-r from-transparent via-ide-border to-transparent" />
            </div>

            <div className="bg-ide-panel/5 border border-ide-border rounded-2xl overflow-hidden shadow-sm">
              {filteredSettings.behavior.some(s => s.id === "formatOnSave") && (
                <SettingRow 
                  label="Formatar ao Salvar" 
                  description="Aciona o formatador Zenith oficial em cada evento de persistência de arquivo."
                >
                  <ToggleSwitch 
                    checked={settings.editorFormatOnSave} 
                    onChange={(val) => updateSettings({ editorFormatOnSave: val })} 
                  />
                </SettingRow>
              )}
              {filteredSettings.behavior.some(s => s.id === "wordWrap") && (
                <SettingRow 
                  label="Quebra de Linha Automática" 
                  description="Impede scroll horizontal ao ajustar o texto dinamicamente à largura da viewport."
                >
                  <ToggleSwitch 
                    checked={settings.editorWordWrap} 
                    onChange={(val) => updateSettings({ editorWordWrap: val })} 
                  />
                </SettingRow>
              )}
            </div>

            {filteredSettings.behavior.some(s => s.id === "cursor") && (
              <div className="p-5 rounded-2xl border border-ide-border bg-ide-panel/5 space-y-5">
                <div className="flex items-center gap-3">
                  <MousePointer size={16} className="text-primary" />
                  <h3 className="text-[13px] font-bold text-ide-text">Estilo de Animação do Cursor</h3>
                </div>
                <div className="grid grid-cols-2 sm:grid-cols-5 gap-2">
                  {['blink', 'smooth', 'phase', 'expand', 'solid'].map((style) => (
                    <button 
                      key={style}
                      onClick={() => updateSettings({ editorCursorBlinking: style as any })}
                      className={`px-3 py-2.5 rounded-xl border text-[10px] font-bold uppercase tracking-wider transition-all ${
                        settings.editorCursorBlinking === style 
                          ? 'border-primary/40 bg-primary/5 text-primary shadow-sm' 
                          : 'border-ide-border bg-ide-bg text-ide-text-dim/60 hover:text-ide-text'
                      }`}
                    >
                      {style}
                    </button>
                  ))}
                </div>
              </div>
            )}
          </section>
        )}

        {/* Visuals Section */}
        {hasVisuals && (
          <section className="space-y-6">
            <div className="flex items-center gap-4 px-2">
              <div className="h-px flex-1 bg-gradient-to-r from-transparent via-ide-border to-transparent" />
              <div className="text-[10px] font-bold uppercase tracking-[0.2em] text-ide-text-dim/40 italic">Ajudas Visuais</div>
              <div className="h-px flex-1 bg-gradient-to-r from-transparent via-ide-border to-transparent" />
            </div>

            <div className="bg-ide-panel/5 border border-ide-border rounded-2xl overflow-hidden shadow-sm">
              {filteredSettings.visuals.some(s => s.id === "minimap") && (
                <SettingRow 
                  label="Exibir Minimapa" 
                  description="Renderiza uma visão estrutural em miniatura no lado direito do buffer."
                >
                  <ToggleSwitch 
                    checked={settings.editorMinimap} 
                    onChange={(val) => updateSettings({ editorMinimap: val })} 
                  />
                </SettingRow>
              )}
            </div>
          </section>
        )}
      </div>
    </div>
  );
}

function SettingRow({ label, description, children }: { label: string, description: string, children: React.ReactNode }) {
  return (
    <div className="flex items-center justify-between px-6 py-5 border-b border-ide-border/40 last:border-0 hover:bg-white/5 transition-all">
      <div className="flex flex-col gap-1.5 max-w-[75%]">
        <span className="text-[13px] font-bold text-ide-text">{label}</span>
        <span className="text-[11px] text-ide-text-dim/60 leading-relaxed">{description}</span>
      </div>
      <div className="flex shrink-0 items-center justify-center">
        {children}
      </div>
    </div>
  );
}
