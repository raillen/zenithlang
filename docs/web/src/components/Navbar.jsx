import React from 'react';
import { motion } from 'framer-motion';
import MagneticWrapper from './MagneticWrapper';

const Navbar = ({ activeSection, setActiveSection }) => {
  const navItems = [
    { id: 'home', label: 'Início' },
    { id: 'handbook', label: 'Docs' },
    { id: 'api', label: 'API' },
    { id: 'pedagogy', label: 'Aprender' },
    { id: 'architecture', label: 'Arquitetura' },
    { id: 'roadmap', label: 'Roadmap' },
    { id: 'changelog', label: 'Changelog' },
  ];

  return (
    <nav 
      data-z-id="navbar-container" 
      className="w-full glass-premium sticky top-0 z-50 py-3 px-8 flex justify-between items-center transition-all"
      style={{ 
        backdropFilter: 'blur(40px) saturate(180%)', 
        WebkitBackdropFilter: 'blur(40px) saturate(180%)',
        backgroundColor: 'rgba(255, 255, 255, 0.4)',
        borderBottom: '1px solid rgba(0, 0, 0, 0.05)'
      }}
    >
      {/* <!-- LOGO: Site Logo --> */}
      <div data-z-id="navbar-logo-container" className="navbar-logo flex items-center gap-3 cursor-pointer" onClick={() => setActiveSection('home')}>
        <img data-z-id="navbar-logo-img" src="/logo-with-text.svg" alt="Zenith" className="h-10 w-auto" />
      </div>
      
      {/* <!-- NAV: Menu Items --> */}
      <ul data-z-id="navbar-menu" className="navbar-menu flex items-center gap-1">
        {navItems.map((item) => (
          <li data-z-id={`navbar-item-wrapper-${item.id}`} key={item.id}>
            <MagneticWrapper strength={0.3}>
              <button
                data-z-id={`navbar-item-btn-${item.id}`}
                onClick={() => setActiveSection(item.id)}
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
    </nav>
  );
};

export default Navbar;