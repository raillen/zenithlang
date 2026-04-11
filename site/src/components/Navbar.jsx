import React, { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { List } from '@phosphor-icons/react';
import MagneticWrapper from './MagneticWrapper';
import MobileMenu from './MobileMenu';
import { getDocsSections } from '../data/docs';
import LanguageSwitcher from './LanguageSwitcher';

import { useLanguage } from '../contexts/LanguageContext';

const Navbar = ({ activeSection, setActiveSection }) => {
  const { lang } = useLanguage();
  const [isMenuOpen, setIsMenuOpen] = useState(false);
  const logoUrl = `${import.meta.env.BASE_URL}logo-with-text.svg`;

  const labels = {
    pt: { home: 'Início', health: 'Saúde' },
    en: { home: 'Home', health: 'Health' },
    ja: { home: 'ホーム', health: 'ヘルス' },
    es: { home: 'Inicio', health: 'Salud' }
  };

  const navItems = [
    { id: 'home', label: labels[lang].home },
    ...getDocsSections(lang),
    { id: 'health', label: labels[lang].health },
  ];

  return (
    <>
      <nav 
        data-z-id="navbar-container" 
        className="w-full glass-premium sticky top-0 z-50 py-3 px-6 md:px-8 flex justify-between items-center transition-all"
        style={{ 
          backdropFilter: 'blur(40px) saturate(180%)', 
          WebkitBackdropFilter: 'blur(40px) saturate(180%)',
          backgroundColor: 'rgba(255, 255, 255, 0.4)',
          borderBottom: '1px solid rgba(0, 0, 0, 0.05)'
        }}
      >
        <div className="flex items-center gap-8">
          {/* <!-- LOGO: Site Logo --> */}
          <div 
            data-z-id="navbar-logo-container" 
            className="navbar-logo flex items-center gap-3 cursor-pointer focus-visible:outline-none" 
            onClick={() => setActiveSection('home')}
            role="button"
            tabIndex={0}
            aria-label="Zenith Home - Voltar para o início"
            onKeyDown={(e) => e.key === 'Enter' && setActiveSection('home')}
          >
            <img data-z-id="navbar-logo-img" src={logoUrl} alt="Zenith Logo" className="h-8 md:h-10 w-auto" />
          </div>
          
          {/* <!-- NAV: Menu Items (Desktop) --> */}
          <ul data-z-id="navbar-menu" className="navbar-menu hidden lg:flex items-center gap-1.5" role="menubar">
            {navItems.map((item) => (
              <li data-z-id={`navbar-item-wrapper-${item.id}`} key={item.id} role="none">
                <MagneticWrapper strength={0.3}>
                  <button
                    data-z-id={`navbar-item-btn-${item.id}`}
                    onClick={() => setActiveSection(item.id)}
                    role="menuitem"
                    aria-current={activeSection === item.id ? 'page' : undefined}
                    className={`navbar-item px-3 py-1.5 text-sm font-semibold rounded-full transition-all ${
                      activeSection === item.id 
                        ? 'bg-primary/10 text-primary' 
                        : 'text-neutral/60 hover:text-neutral hover:bg-black/5'
                    }`}
                  >
                    {item.label}
                  </button>
                </MagneticWrapper>
              </li>
            ))}
          </ul>
        </div>

        <div className="flex items-center gap-4">
          <LanguageSwitcher />
        </div>

        {/* <!-- MOBILE: Menu Toggle --> */}
        <button 
          data-z-id="navbar-mobile-toggle"
          onClick={() => setIsMenuOpen(true)}
          className="flex md:hidden p-2 rounded-full hover:bg-black/5 text-neutral/60 transition-colors"
          aria-label="Abrir Menu de Navegação"
          aria-expanded={isMenuOpen}
          aria-haspopup="true"
        >
          <List size={24} weight="bold" />
        </button>
      </nav>

      {/* <!-- MOBILE: Menu Overlay --> */}
      <AnimatePresence>
        {isMenuOpen && (
          <MobileMenu 
            isOpen={isMenuOpen} 
            onClose={() => setIsMenuOpen(false)}
            navItems={navItems}
            activeSection={activeSection}
            setActiveSection={setActiveSection}
          />
        )}
      </AnimatePresence>
    </>
  );
};

export default Navbar;
