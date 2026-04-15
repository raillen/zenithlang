import React, { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { Globe } from '@phosphor-icons/react';
import { useLanguage } from '../contexts/LanguageContext';

const GlobeIcon = Globe || (() => <span className="text-lg">🌐</span>);

const LanguageSwitcher = () => {
  const { lang, setLang, languages } = useLanguage();
  const [isOpen, setIsOpen] = useState(false);

  const currentLang = languages.find(l => l.id === lang) || languages[0];

  return (
    <div className="relative">
      <button
        onClick={() => setIsOpen(!isOpen)}
        className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-black/5 hover:bg-black/10 transition-all text-sm font-medium border border-black/5"
      >
        <GlobeIcon size={16} weight="bold" className="text-primary/70" />
        <span className="opacity-60">{currentLang.id.toUpperCase()}</span>
        <span className="text-[10px] opacity-30">▼</span>
      </button>

      <AnimatePresence>
        {isOpen && (
          <>
            <div 
              className="fixed inset-0 z-40" 
              onClick={() => setIsOpen(false)}
            />
            <motion.div
              initial={{ opacity: 0, y: 10, scale: 0.95 }}
              animate={{ opacity: 1, y: 0, scale: 1 }}
              exit={{ opacity: 0, y: 10, scale: 0.95 }}
              className="absolute right-0 mt-2 w-40 glass-premium z-50 rounded-2xl overflow-hidden shadow-2xl border border-white/20"
              style={{ backdropFilter: 'blur(20px)' }}
            >
              {languages.map((l) => (
                <button
                  key={l.id}
                  onClick={() => {
                    setLang(l.id);
                    setIsOpen(false);
                  }}
                  className={`w-full text-left px-4 py-3 text-sm transition-colors flex justify-between items-center ${
                    lang === l.id 
                      ? 'bg-primary/10 text-primary font-bold' 
                      : 'hover:bg-black/5 text-neutral/70'
                  }`}
                >
                  {l.label}
                  {lang === l.id && <span className="w-1.5 h-1.5 rounded-full bg-primary" />}
                </button>
              ))}
            </motion.div>
          </>
        )}
      </AnimatePresence>
    </div>
  );
};

export default LanguageSwitcher;
