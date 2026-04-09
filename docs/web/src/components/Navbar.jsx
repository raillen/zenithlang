import React from 'react';
import { motion } from 'framer-motion';

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
    <nav data-z-id="navbar-container" className="navbar w-full bg-[#ECEEEE]/80 backdrop-blur-md border-b border-black/5 py-4 px-6 flex justify-between items-center transition-all">
      {/* <!-- LOGO: Site Logo --> */}
      <div data-z-id="navbar-logo-container" className="navbar-logo flex items-center gap-3 cursor-pointer" onClick={() => setActiveSection('home')}>
        <img data-z-id="navbar-logo-img" src="/logo-with-text.svg" alt="Zenith" className="h-10 w-auto" />
      </div>
      
      {/* <!-- NAV: Menu Items --> */}
      <ul data-z-id="navbar-menu" className="navbar-menu flex items-center gap-1">
        {navItems.map((item) => (
          <li data-z-id={`navbar-item-wrapper-${item.id}`} key={item.id}>
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
          </li>
        ))}
      </ul>
    </nav>
  );
};

export default Navbar;