import React from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { 
  Files, MagnifyingGlass, GitBranch, Cube, 
  Gear, User, Package, Layout, Palette, Lightning, 
  ShieldCheck, ArrowUpRight, CaretDown 
} from '@phosphor-icons/react';

const IdePreview = ({ theme, showHub = false }) => {
  const { ui, syntax } = theme;

  // CSS variables for the mock instance
  const style = {
    '--bg': ui.background,
    '--panel': ui.panel,
    '--border': ui.border,
    '--text': ui.textMain,
    '--muted': ui.textMuted,
    '--primary': ui.primary,
    '--kw': syntax.keyword,
    '--str': syntax.string,
    '--fn': syntax.function,
    '--num': syntax.number,
    '--type': syntax.type,
    '--comment': syntax.comment,
  };

  return (
    <div 
      className="relative w-full h-full rounded-2xl overflow-hidden shadow-2xl border flex font-sans select-none"
      style={{ 
        ...style, 
        backgroundColor: 'var(--bg)', 
        color: 'var(--text)',
        borderColor: 'var(--border)' 
      }}
    >
      {/* ACTIVITY BAR */}
      <aside className="w-12 border-r flex flex-col items-center py-4 gap-4" style={{ borderColor: 'var(--border)', backgroundColor: 'var(--panel)' }}>
        <Files size={20} weight="fill" className="opacity-80" style={{ color: 'var(--primary)' }} />
        <MagnifyingGlass size={20} className="opacity-40" />
        <GitBranch size={20} className="opacity-40" />
        <Cube size={20} className="opacity-40" />
        <div className="flex-1" />
        <User size={20} className="opacity-40" />
        <Gear size={20} className="opacity-40" />
      </aside>

      {/* SIDEBAR */}
      <aside className="w-48 border-r hidden md:flex flex-col" style={{ borderColor: 'var(--border)', backgroundColor: 'var(--panel)' }}>
        <header className="h-9 flex items-center px-4 border-b text-[10px] font-bold uppercase tracking-widest opacity-40" style={{ borderColor: 'var(--border)' }}>
          Explorer
        </header>
        <div className="p-4 space-y-3">
          <div className="flex items-center gap-2 text-xs opacity-80">
            <Files size={14} />
            <span>src/main.zt</span>
          </div>
          <div className="flex items-center gap-2 text-xs opacity-40">
            <Files size={14} />
            <span>lib/core.zt</span>
          </div>
          <div className="flex items-center gap-2 text-xs opacity-40">
            <Package size={14} />
            <span>zenith.json</span>
          </div>
        </div>
      </aside>

      {/* EDITOR AREA */}
      <main className="flex-1 flex flex-col min-w-0">
        <header className="h-9 flex items-center border-b" style={{ borderColor: 'var(--border)', backgroundColor: 'var(--panel)' }}>
          <div className="px-4 h-full flex items-center gap-2 border-r text-[11px] font-bold" style={{ borderColor: 'var(--border)', backgroundColor: 'var(--bg)' }}>
             <Lightning size={12} style={{ color: 'var(--primary)' }} />
             main.zt
          </div>
        </header>
        
        <div className="flex-1 p-6 font-mono text-[13px] leading-relaxed overflow-hidden">
          <CodeLine num="01" kw="fn" fn="fibonacci" rest="(n: i32) -> i32 {" />
          <CodeLine num="02" indent kw="if" rest="n <= " numVal="1" rest2=" { " />
          <CodeLine num="03" indent={2} kw="return" rest=" n" />
          <CodeLine num="04" indent kw="}" />
          <CodeLine num="05" indent kw="return" fn=" fibonacci" rest="(n - " numVal="1" rest2=") + " fn2="fibonacci" rest3="(n - " numVal2="2" rest4=")" />
          <CodeLine num="06" rest="}" />
          <div className="mt-4 opacity-30 select-none">
             <span style={{ color: 'var(--comment)' }}>// Teste seu tema soberano agora.</span>
          </div>
        </div>

        {/* TERMINAL MOCKUP */}
        <div className="h-32 border-t p-4 flex flex-col gap-2" style={{ borderColor: 'var(--border)', backgroundColor: 'var(--panel)' }}>
           <div className="flex items-center gap-4 border-b pb-2 mb-1" style={{ borderColor: 'var(--border)' }}>
              <span className="text-[10px] font-bold uppercase tracking-widest opacity-80">Terminal</span>
              <span className="text-[10px] uppercase opacity-30">Output</span>
              <span className="text-[10px] uppercase opacity-30">Debug</span>
           </div>
           <div className="font-mono text-[11px] space-y-1">
              <div className="flex gap-2">
                 <span style={{ color: 'var(--primary)' }}>$</span>
                 <span>zenith build . --release</span>
              </div>
              <div style={{ color: 'var(--fn)' }}>Compiling main.zt [Stage 1]...</div>
              <div className="opacity-40">Build success in 42ms.</div>
           </div>
        </div>
      </main>

      {/* STATUS BAR */}
      <footer className="absolute bottom-0 left-0 right-0 h-6 flex items-center px-4 justify-between text-[10px] font-bold uppercase tracking-widest" style={{ backgroundColor: 'var(--primary)', color: 'white' }}>
         <div className="flex items-center gap-4">
            <div className="flex items-center gap-1.5">
               <GitBranch size={10} weight="bold" />
               <span>master</span>
            </div>
            <div className="flex items-center gap-1.5">
               <ShieldCheck size={10} weight="bold" />
               <span>Sovereign Guard</span>
            </div>
         </div>
         <div className="flex items-center gap-4">
            <span>UTF-8</span>
            <span>Zenith 0.3.0</span>
         </div>
      </footer>

      {/* HUB OVERLAY */}
      <AnimatePresence>
        {showHub && (
          <motion.div 
            initial={{ opacity: 0, scale: 1.05 }}
            animate={{ opacity: 1, scale: 1 }}
            exit={{ opacity: 0, scale: 1.05 }}
            className="absolute inset-0 z-50 flex items-center justify-center p-8 bg-black/40 backdrop-blur-sm"
          >
            <div 
              className="w-full h-full max-w-2xl max-h-[450px] rounded-3xl border shadow-[0_32px_64px_-16px_rgba(0,0,0,0.5)] flex overflow-hidden"
              style={{ 
                backgroundColor: 'var(--panel)', 
                borderColor: 'var(--border)',
                backdropFilter: ui.glass ? 'blur(20px)' : 'none'
              }}
            >
              {/* HUB SIDEBAR */}
              <aside className="w-40 border-r flex flex-col" style={{ borderColor: 'var(--border)', backgroundColor: 'rgba(0,0,0,0.1)' }}>
                <div className="p-6">
                   <div className="h-6 w-6 rounded-lg bg-primary/20 flex items-center justify-center mb-6">
                      <Lightning size={14} style={{ color: 'var(--primary)' }} />
                   </div>
                   <nav className="space-y-4">
                      <NavItem icon={<Palette size={14} />} label="Theme Lab" active />
                      <NavItem icon={<Package size={14} />} label="Extensions" />
                      <NavItem icon={<Layout size={14} />} label="Editor" />
                      <NavItem icon={<Gear size={14} />} label="General" />
                   </nav>
                </div>
              </aside>

              {/* HUB CONTENT */}
              <main className="flex-1 p-8 flex flex-col gap-6 overflow-hidden">
                <header>
                   <div className="text-[10px] font-black uppercase tracking-[.3em] opacity-40 mb-2">Workspace Studio</div>
                   <h2 className="text-xl font-bold">Zenith Hub Premium</h2>
                </header>
                
                <div className="grid grid-cols-2 gap-4">
                   <HubCard title="Official Themes" icon={<Palette size={16} />} color="var(--primary)" />
                   <HubCard title="Marketplace" icon={<ArrowUpRight size={16} />} color="var(--fn)" />
                </div>

                <div className="flex-1 rounded-2xl border bg-white/5 p-4 flex flex-col gap-4" style={{ borderColor: 'var(--border)' }}>
                   <div className="flex items-center justify-between">
                      <span className="text-xs font-bold opacity-60">Contrast Shield</span>
                      <div className="h-1.5 w-6 rounded-full bg-emerald-500/40" />
                   </div>
                   <div className="h-2 w-full bg-white/5 rounded-full" />
                   <div className="h-2 w-2/3 bg-white/5 rounded-full" />
                </div>
              </main>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  );
};

// Sub-components for tidier code
const CodeLine = ({ num, indent = 0, kw, fn, rest, numVal, rest2, fn2, rest3, numVal2, rest4 }) => (
  <div className="flex gap-4">
    <span className="w-5 text-right opacity-20">{num}</span>
    <div className={`flex flex-wrap gap-x-1 ${indent === 1 ? 'ml-4' : indent === 2 ? 'ml-8' : ''}`}>
      {kw && <span style={{ color: 'var(--kw)', fontWeight: 'bold' }}>{kw}</span>}
      {fn && <span style={{ color: 'var(--fn)' }}>{fn}</span>}
      {rest && <span className="opacity-80">{rest}</span>}
      {numVal && <span style={{ color: 'var(--num)' }}>{numVal}</span>}
      {rest2 && <span className="opacity-80">{rest2}</span>}
      {fn2 && <span style={{ color: 'var(--fn)' }}>{fn2}</span>}
      {rest3 && <span className="opacity-80">{rest3}</span>}
      {numVal2 && <span style={{ color: 'var(--num)' }}>{numVal2}</span>}
      {rest4 && <span className="opacity-80">{rest4}</span>}
    </div>
  </div>
);

const NavItem = ({ icon, label, active }) => (
  <div 
    className={`flex items-center gap-3 px-3 py-2 rounded-xl text-[11px] font-bold transition-all ${
      active ? 'bg-primary/10' : 'opacity-40 hover:opacity-100 hover:bg-white/5'
    }`}
    style={{ color: active ? 'var(--primary)' : 'inherit' }}
  >
    {icon}
    <span>{label}</span>
  </div>
);

const HubCard = ({ title, icon, color }) => (
  <div 
    className="p-4 rounded-2xl border bg-white/5 flex flex-col gap-3 group hover:bg-white/[0.08] transition-all cursor-pointer"
    style={{ borderColor: 'var(--border)' }}
  >
     <div className="h-8 w-8 rounded-lg flex items-center justify-center bg-black/20" style={{ color }}>
        {icon}
     </div>
     <span className="text-[11px] font-bold">{title}</span>
  </div>
);

export default IdePreview;
