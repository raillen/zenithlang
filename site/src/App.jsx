import React, { useState, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import Navbar from './components/Navbar';
import Home from './pages/Home';
import DocViewer from './pages/DocViewer';
import Health from './pages/Health';
import SearchModal from './components/SearchModal';
import Footer from './components/Footer';
import { LanguageProvider } from './contexts/LanguageContext';

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
    <LanguageProvider>
      <div data-z-id="app-root" className="min-h-screen bg-[var(--color-surface)] selection:bg-primary selection:text-white">
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
          ) : activeSection === 'health' ? (
            <motion.div
              key="health"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -20 }}
              className="w-full"
            >
              <Health />
            </motion.div>
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

      <Footer onNavigate={setActiveSection} />
      </div>
    </LanguageProvider>
  );
}

export default App;
