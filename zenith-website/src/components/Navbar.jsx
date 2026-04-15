import React, { useState, useRef, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { List, CaretDown, Palette, Notebook } from '@phosphor-icons/react';
import MagneticWrapper from './MagneticWrapper';
import MobileMenu from './MobileMenu';
import { getDocsSections, getAllDocs } from '../data/docs';
import LanguageSwitcher from './LanguageSwitcher';
import { useLanguage } from '../contexts/LanguageContext';

const Navbar = ({ activeSection, setActiveSection }) => {
  const languageContext = useLanguage();
  const lang = languageContext?.lang || 'pt';
  const [isMenuOpen, setIsMenuOpen] = useState(false);
  const [activeDropdown, setActiveDropdown] = useState(null);
  const closeTimeout = useRef(null);
  const logoUrl = `${import.meta.env.BASE_URL}logo-with-text.svg`;

  const labels = {
    pt: { home: 'Início', vision: 'Visão', language: 'Linguagem', ecosystem: 'Ecossistema' },
    en: { home: 'Home', vision: 'Vision', language: 'Language', ecosystem: 'Ecosystem' },
    ja: { home: 'ホーム', vision: 'ビジョン', language: '言語', ecosystem: 'エコシステム' },
    es: { home: 'Inicio', vision: 'Visión', language: 'Lenguaje', ecosystem: 'Ecosistema' }
  };

  const currentLabels = labels[lang] || labels.pt;
  const allSections = getDocsSections(lang);
  const allDocs = getAllDocs(lang);
  
  // Find specific docs for explicit menu entry
  const manifestoDoc = allDocs.find(d => d.file === 'MANIFESTO.md');

  const categories = [
    {
      id: 'vision',
      label: currentLabels.vision,
      items: [
        manifestoDoc ? { id: 'manifesto', label: 'O Manifesto', file: manifestoDoc.file } : null,
        allSections.find(s => s.id === 'start'),
        allSections.find(s => s.id === 'roadmap'),
      ].filter(Boolean)
    },
    {
      id: 'language',
      label: currentLabels.language,
      items: [
        allSections.find(s => s.id === 'language'),
        allSections.find(s => s.id === 'learn'),
        allSections.find(s => s.id === 'reference'),
      ].filter(Boolean)
    },
    {
      id: 'ecosystem',
      label: currentLabels.ecosystem,
      items: [
        allSections.find(s => s.id === 'compiler'),
        { id: 'theme', label: 'Theme Lab Studio' },
        allSections.find(s => s.id === 'support'),
      ].filter(Boolean)
    }
  ];

  const handleMouseEnter = (id) => {
    if (closeTimeout.current) {
      clearTimeout(closeTimeout.current);
      closeTimeout.current = null;
    }
    setActiveDropdown(id);
  };

  const handleMouseLeave = () => {
    closeTimeout.current = setTimeout(() => {
      setActiveDropdown(null);
    }, 250);
  };

  const handleNavigate = (id) => {
    // If it's the manifesto, we use a special ID or just trigger the section
    if (id === 'manifesto') {
      setActiveSection('start'); // Manifesto is in 'start' section
    } else {
      setActiveSection(id);
    }
    setActiveDropdown(null);
    if (closeTimeout.current) clearTimeout(closeTimeout.current);
  };

  return (
    <>
      <nav 
        className="w-full glass-premium sticky top-0 z-50 py-3 px-6 md:px-8 flex justify-between items-center transition-all h-[72px]"
        style={{ 
          backdropFilter: 'blur(40px) saturate(180%)', 
          WebkitBackdropFilter: 'blur(40px) saturate(180%)',
          backgroundColor: 'rgba(255, 255, 255, 0.6)',
          borderBottom: '1px solid rgba(0, 0, 0, 0.05)'
        }}
      >
        <div className="flex items-center gap-8 h-full">
          <div 
            className="navbar-logo flex items-center gap-3 cursor-pointer" 
            onClick={() => handleNavigate('home')}
          >
            <img src={logoUrl} alt="Zenith Logo" className="h-8 md:h-10 w-auto" />
          </div>
          
          <ul className="hidden lg:flex items-center gap-1 h-full" onMouseLeave={handleMouseLeave}>
            {categories.map((cat) => {
              const isChildActive = cat.items.some(it => it.id === activeSection || (it.id === 'manifesto' && activeSection === 'start'));
              
              return (
                <li 
                  key={cat.id} 
                  className="relative flex items-center h-full px-1 group"
                  onMouseEnter={() => handleMouseEnter(cat.id)}
                >
                  <MagneticWrapper strength={0.1}>
                    <button
                      className={`flex items-center gap-2 px-4 py-2 text-sm font-bold rounded-full transition-all ${
                        isChildActive 
                          ? 'bg-primary/20 text-primary' 
                          : 'text-neutral/50 hover:text-neutral hover:bg-black/5'
                      }`}
                    >
                      {cat.label}
                      <CaretDown size={14} weight="bold" className={`transition-transform duration-300 ${activeDropdown === cat.id ? 'rotate-180' : ''}`} />
                    </button>
                  </MagneticWrapper>

                  <AnimatePresence>
                    {activeDropdown === cat.id && (
                      <motion.div
                        onMouseEnter={() => handleMouseEnter(cat.id)}
                        initial={{ opacity: 0, y: 10, scale: 0.95 }}
                        animate={{ opacity: 1, y: 0, scale: 1 }}
                        exit={{ opacity: 0, y: 10, scale: 0.95 }}
                        transition={{ duration: 0.2, ease: "easeOut" }}
                        className="absolute left-1/2 -translate-x-1/2 top-full w-64 z-[60]"
                      >
                        {/* THE HOVER BRIDGE: Essential to prevent closure between button and menu */}
                        <div className="absolute -top-10 left-0 right-0 h-12 bg-transparent cursor-default" />
                        
                        <div className="relative bg-white/95 backdrop-blur-2xl rounded-2xl border border-black/5 shadow-2xl p-2 mt-4 overflow-hidden">
                          {cat.items.map((item) => (
                            <button
                              key={item.id}
                              onClick={() => handleNavigate(item.id)}
                              className={`w-full text-left px-4 py-3 rounded-xl text-[13px] font-bold transition-all flex items-center justify-between group/item ${
                                activeSection === item.id || (item.id === 'manifesto' && activeSection === 'start')
                                  ? 'bg-primary/5 text-primary' 
                                  : 'text-neutral/60 hover:bg-black/5 hover:text-neutral'
                              }`}
                            >
                              <div className="flex items-center gap-3">
                                {item.id === 'manifesto' && <Notebook size={16} className="text-primary/60" />}
                                <span>{item.label}</span>
                              </div>
                              {item.id === 'theme' && <Palette size={14} className="opacity-40" />}
                            </button>
                          ))}
                        </div>
                      </motion.div>
                    )}
                  </AnimatePresence>
                </li>
              );
            })}
          </ul>
        </div>

        <div className="flex items-center gap-4">
          <LanguageSwitcher />
        </div>

        <button 
          onClick={() => setIsMenuOpen(true)}
          className="flex lg:hidden p-2 rounded-full hover:bg-black/5 text-neutral/60 transition-colors"
        >
          <List size={24} weight="bold" />
        </button>
      </nav>

      <AnimatePresence mode="wait">
        {isMenuOpen && (
          <MobileMenu 
            isOpen={isMenuOpen} 
            onClose={() => setIsMenuOpen(false)}
            categories={categories}
            activeSection={activeSection}
            setActiveSection={setActiveSection}
            labels={currentLabels}
          />
        )}
      </AnimatePresence>
    </>
  );
};

export default Navbar;
