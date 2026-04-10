import React, { useState, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import Navbar from './components/Navbar';
import Home from './pages/Home';
import DocViewer from './pages/DocViewer';
import SearchModal from './components/SearchModal';
import CustomCursor from './components/CustomCursor';

function App() {
  const [activeSection, setActiveSection] = useState('home');
  const [isSearchOpen, setIsSearchOpen] = useState(false);
  const [requestedDoc, setRequestedDoc] = useState(null);

  // Global Ctrl+K Listener
  useEffect(() => {
    const handleKeyDown = (e) => {
      if ((e.ctrlKey || e.metaKey) && e.key === 'k') {
        e.preventDefault();
        setIsSearchOpen(true);
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);

  const handleSearchSelect = (doc) => {
    setActiveSection(doc.section);
    setRequestedDoc(doc);
    setIsSearchOpen(false);
  };

  return (
    <div data-z-id="app-root" className="min-h-screen bg-[#ECEEEE] selection:bg-primary selection:text-white cursor-none">
      <CustomCursor />
      {/* <!-- HEADER: Navbar --> */}
      <header data-z-id="app-header" className="site-header-navbar sticky top-0 z-50 w-full">
        <Navbar activeSection={activeSection} setActiveSection={setActiveSection} />
      </header>
      
      {/* <!-- MODAL: Search Modal --> */}
      <section data-z-id="app-search-modal-container" className="site-modal-search">
        <SearchModal 
          isOpen={isSearchOpen} 
          onClose={() => setIsSearchOpen(false)} 
          onSelect={handleSearchSelect} 
        />
      </section>

      {/* <!-- MAIN: Page Content --> */}
      <main data-z-id="app-main-content" className="site-main-content relative">
        <AnimatePresence mode="wait">
          {activeSection === 'home' ? (
            <Home key="home" onNavigate={setActiveSection} />
          ) : (
            <motion.div
              data-z-id="app-docviewer-wrapper"
              key={activeSection}
              initial={{ opacity: 0, x: 20 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -20 }}
              transition={{ duration: 0.2 }}
              className="w-full"
            >
              <DocViewer section={activeSection} requestedDoc={requestedDoc} />
            </motion.div>
          )}
        </AnimatePresence>
      </main>

      {/* <!-- FOOTER: Site Footer --> */}
      <footer data-z-id="app-footer" className="site-footer py-8 px-6 mt-10 border-t border-black/5 text-center">
        <div data-z-id="app-footer-container" className="max-w-5xl mx-auto flex flex-col md:flex-row items-center justify-between gap-4">
          <div data-z-id="app-footer-branding" className="flex items-center gap-3 text-neutral/50 font-medium text-sm">
             <img data-z-id="app-footer-logo" src="/logo-only.svg" alt="Zenith" className="h-5 w-5 opacity-50 grayscale" />
             <span data-z-id="app-footer-copyright">© 2026 Zenith. Desenvolvido com Antigravity.</span>
          </div>
          <nav data-z-id="app-footer-nav" className="flex gap-6 text-sm font-medium text-neutral/40">
            <a data-z-id="app-footer-link-docs" href="#" className="hover:text-neutral transition-colors">Documentação</a>
            <a data-z-id="app-footer-link-github" href="#" className="hover:text-neutral transition-colors">GitHub</a>
            <a data-z-id="app-footer-link-discord" href="#" className="hover:text-neutral transition-colors">Discord</a>
          </nav>
        </div>
      </footer>
    </div>
  );
}

export default App;