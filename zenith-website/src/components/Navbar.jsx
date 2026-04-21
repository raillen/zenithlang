import React, { useState, useRef } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { List, CaretDown } from '@phosphor-icons/react';
import MagneticWrapper from './MagneticWrapper';
import MobileMenu from './MobileMenu';
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
    pt: { 
      home: 'Início',
      docs: 'Docs',
      narrative: 'Narrativa',
      roadmap: 'Roadmap',
      learn: 'Aprender',
      reference: 'Referência'
    },
    en: { 
      home: 'Home',
      docs: 'Docs',
      narrative: 'Narrative',
      roadmap: 'Roadmap',
      learn: 'Learn',
      reference: 'Reference'
    },
    ja: { 
      home: 'ホーム',
      docs: 'ドキュメント',
      narrative: 'ナラティブ',
      roadmap: 'ロードマップ',
      learn: '学ぶ',
      reference: 'リファレンス'
    },
    es: { 
      home: 'Inicio',
      docs: 'Docs',
      narrative: 'Narrativa',
      roadmap: 'Roadmap',
      learn: 'Aprender',
      reference: 'Referencia'
    }
  };

  const currentLabels = labels[lang] || labels.pt;

  const docsItems = [
    { id: 'narrative', label: currentLabels.narrative },
    { id: 'roadmap', label: currentLabels.roadmap },
    { id: 'learn', label: currentLabels.learn },
    { id: 'reference', label: currentLabels.reference },
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
    setActiveSection(id);
    setActiveDropdown(null);
    if (closeTimeout.current) clearTimeout(closeTimeout.current);
  };

  const isDocsActive = ['narrative', 'roadmap', 'learn', 'reference'].includes(activeSection);

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
        {/* Logo à esquerda */}
        <div 
          className="navbar-logo flex items-center gap-3 cursor-pointer" 
          onClick={() => handleNavigate('home')}
        >
          <img src={logoUrl} alt="Zenith Logo" className="h-8 md:h-10 w-auto" />
        </div>
        
        {/* Menu à direita */}
        <div className="flex items-center gap-4">
          {/* Desktop Menu */}
          <ul className="hidden lg:flex items-center gap-1 h-full" onMouseLeave={handleMouseLeave}>
            {/* Home */}
            <li className="relative flex items-center h-full px-1">
              <MagneticWrapper strength={0.1}>
                <button
                  onClick={() => handleNavigate('home')}
                  className={`flex items-center gap-2 px-4 py-2 text-sm font-bold rounded-full transition-all ${
                    activeSection === 'home'
                      ? 'bg-primary/20 text-primary' 
                      : 'text-neutral/50 hover:text-neutral hover:bg-black/5'
                  }`}
                >
                  {currentLabels.home}
                </button>
              </MagneticWrapper>
            </li>

            {/* Docs Dropdown */}
            <li 
              className="relative flex items-center h-full px-1 group"
              onMouseEnter={() => handleMouseEnter('docs')}
            >
              <MagneticWrapper strength={0.1}>
                <button
                  className={`flex items-center gap-2 px-4 py-2 text-sm font-bold rounded-full transition-all ${
                    isDocsActive
                      ? 'bg-primary/20 text-primary' 
                      : 'text-neutral/50 hover:text-neutral hover:bg-black/5'
                  }`}
                >
                  {currentLabels.docs}
                  <CaretDown size={14} weight="bold" className={`transition-transform duration-300 ${activeDropdown === 'docs' ? 'rotate-180' : ''}`} />
                </button>
              </MagneticWrapper>

              <AnimatePresence>
                {activeDropdown === 'docs' && (
                  <motion.div
                    onMouseEnter={() => handleMouseEnter('docs')}
                    initial={{ opacity: 0, y: 10, scale: 0.95 }}
                    animate={{ opacity: 1, y: 0, scale: 1 }}
                    exit={{ opacity: 0, y: 10, scale: 0.95 }}
                    transition={{ duration: 0.2, ease: "easeOut" }}
                    className="absolute right-0 top-full w-64 z-[60]"
                  >
                    {/* Hover bridge */}
                    <div className="absolute -top-10 left-0 right-0 h-12 bg-transparent cursor-default" />
                    
                    <div className="relative bg-white/95 backdrop-blur-2xl rounded-2xl border border-black/5 shadow-2xl p-2 mt-4 overflow-hidden">
                      {docsItems.map((item) => (
                        <button
                          key={item.id}
                          onClick={() => handleNavigate(item.id)}
                          className={`w-full text-left px-4 py-3 rounded-xl text-[13px] font-bold transition-all ${
                            activeSection === item.id
                              ? 'bg-primary/5 text-primary' 
                              : 'text-neutral/60 hover:bg-black/5 hover:text-neutral'
                          }`}
                        >
                          {item.label}
                        </button>
                      ))}
                    </div>
                  </motion.div>
                )}
              </AnimatePresence>
            </li>
          </ul>

          {/* Language Switcher */}
          <LanguageSwitcher />

          {/* Mobile Menu Button */}
          <button 
            onClick={() => setIsMenuOpen(true)}
            className="flex lg:hidden p-2 rounded-full hover:bg-black/5 text-neutral/60 transition-colors"
          >
            <List size={24} weight="bold" />
          </button>
        </div>
      </nav>

      <AnimatePresence mode="wait">
        {isMenuOpen && (
          <MobileMenu 
            isOpen={isMenuOpen} 
            onClose={() => setIsMenuOpen(false)}
            docsItems={docsItems}
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
