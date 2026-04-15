import React, { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { Code, Repeat } from '@phosphor-icons/react';

const CodeSwitcher = ({ zenithCode, luaCode, title = "Exemplo Prático" }) => {
  const [mode, setMode] = useState('zenith'); // 'zenith' or 'lua'

  return (
    <div className="code-switcher-widget my-12 border border-black/5 rounded-2xl overflow-hidden bg-white shadow-sm transition-all hover:shadow-md">
      {/* Header */}
      <div className="flex items-center justify-between px-6 py-4 border-b border-black/5 bg-[#F9FAFB]">
        <div className="flex items-center gap-3">
          <div className="p-2 bg-primary/10 rounded-lg">
            <Code className="w-5 h-5 text-primary" weight="bold" />
          </div>
          <span className="font-semibold text-neutral text-sm">{title}</span>
        </div>
        
        <button 
          onClick={() => setMode(mode === 'zenith' ? 'lua' : 'zenith')}
          className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-white border border-black/5 text-[11px] font-bold uppercase tracking-wider text-neutral/60 hover:text-primary hover:border-primary/20 transition-all active:scale-95 shadow-sm"
        >
          <Repeat className={`w-3.5 h-3.5 transition-transform duration-500 ${mode === 'lua' ? 'rotate-180' : ''}`} />
          {mode === 'zenith' ? 'Ver em Lua' : 'Ver em Zenith'}
        </button>
      </div>

      {/* Content */}
      <div className="relative min-h-[120px] bg-white p-6 font-mono text-sm leading-relaxed overflow-x-auto">
        <AnimatePresence mode="wait">
          <motion.pre
            key={mode}
            initial={{ opacity: 0, y: 10, filter: 'blur(10px)' }}
            animate={{ opacity: 1, y: 0, filter: 'blur(0px)' }}
            exit={{ opacity: 0, y: -10, filter: 'blur(10px)' }}
            transition={{ duration: 0.3, ease: "easeOut" }}
            className="m-0 text-neutral/80"
          >
            <code>
              {mode === 'zenith' ? zenithCode : luaCode}
            </code>
          </motion.pre>
        </AnimatePresence>

        {/* Badge Indicator */}
        <div className="absolute bottom-4 right-6 pointer-events-none">
          <span className="text-[10px] font-bold uppercase tracking-widest text-primary/30">
            {mode === 'zenith' ? 'Zenith Source' : 'Lua Output'}
          </span>
        </div>
      </div>
    </div>
  );
};

export default CodeSwitcher;
