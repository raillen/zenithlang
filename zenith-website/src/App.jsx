import React, { useState, useEffect, lazy, Suspense } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import Navbar from './components/Navbar';
import Home from './pages/Home';
import DocViewer from './pages/DocViewer';
import ThemeLab from './pages/ThemeLab';
import SearchModal from './components/SearchModal';
import Footer from './components/Footer';
import { LanguageProvider } from './contexts/LanguageContext';

// Lazy-loaded documentation section components
const DocsLanding = lazy(() => import('./pages/docs/DocsLanding'));
const NarrativeSection = lazy(() => import('./pages/docs/NarrativeSection'));
const RoadmapSection = lazy(() => import('./pages/docs/RoadmapSection'));
const LearnSection = lazy(() => import('./pages/docs/LearnSection'));
const ReferenceSection = lazy(() => import('./pages/docs/ReferenceSection'));

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

  const renderContent = () => {
    // Loading fallback component
    const LoadingFallback = () => (
      <div className="flex items-center justify-center min-h-screen">
        <div className="flex flex-col items-center gap-4">
          <div className="w-12 h-12 border-4 border-primary border-t-transparent rounded-full animate-spin" />
          <p className="text-neutral/60 font-medium">Carregando...</p>
        </div>
      </div>
    );

    switch (activeSection) {
      case 'home':
        return <Home key="home" onNavigate={setActiveSection} />;
      
      case 'theme':
        return (
          <motion.div
            key="theme"
            initial={{ opacity: 0, y: 20 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0, y: -20 }}
            className="w-full"
          >
            <ThemeLab />
          </motion.div>
        );
      
      // Documentation sections with lazy loading
      case 'docs':
        return (
          <Suspense fallback={<LoadingFallback />}>
            <motion.div
              key="docs"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -20 }}
              transition={{ duration: 0.3 }}
              className="w-full"
            >
              <DocsLanding onNavigate={setActiveSection} />
            </motion.div>
          </Suspense>
        );
      
      case 'narrative':
        return (
          <Suspense fallback={<LoadingFallback />}>
            <motion.div
              key="narrative"
              initial={{ opacity: 0, x: 20 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -20 }}
              transition={{ duration: 0.3 }}
              className="w-full"
            >
              <NarrativeSection />
            </motion.div>
          </Suspense>
        );
      
      case 'roadmap':
        return (
          <Suspense fallback={<LoadingFallback />}>
            <motion.div
              key="roadmap"
              initial={{ opacity: 0, x: 20 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -20 }}
              transition={{ duration: 0.3 }}
              className="w-full"
            >
              <RoadmapSection />
            </motion.div>
          </Suspense>
        );
      
      case 'learn':
        return (
          <Suspense fallback={<LoadingFallback />}>
            <motion.div
              key="learn"
              initial={{ opacity: 0, x: 20 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -20 }}
              transition={{ duration: 0.3 }}
              className="w-full"
            >
              <LearnSection />
            </motion.div>
          </Suspense>
        );
      
      case 'reference':
        return (
          <Suspense fallback={<LoadingFallback />}>
            <motion.div
              key="reference"
              initial={{ opacity: 0, x: 20 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -20 }}
              transition={{ duration: 0.3 }}
              className="w-full"
            >
              <ReferenceSection />
            </motion.div>
          </Suspense>
        );
      
      default:
        return (
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
        );
    }
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
            {renderContent()}
          </AnimatePresence>
        </main>

        <Footer onNavigate={setActiveSection} />
      </div>
    </LanguageProvider>
  );
}

export default App;
