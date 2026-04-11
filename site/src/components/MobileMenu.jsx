import React from 'react';
import { motion } from 'framer-motion';
import { X } from '@phosphor-icons/react';

const MobileMenu = ({ isOpen, onClose, navItems, activeSection, setActiveSection }) => {
  const logoUrl = `${import.meta.env.BASE_URL}logo-only.svg`;
  // ESC to close
  React.useEffect(() => {
    const handleEsc = (e) => {
      if (e.key === 'Escape' && isOpen) onClose();
    };
    window.addEventListener('keydown', handleEsc);
    return () => window.removeEventListener('keydown', handleEsc);
  }, [isOpen, onClose]);

  const menuVariants = {
    closed: { 
      opacity: 0,
      scale: 1.1,
      transition: { duration: 0.3, ease: "easeInOut" }
    },
    open: { 
      opacity: 1,
      scale: 1,
      transition: { 
        duration: 0.4, 
        ease: "easeOut",
        staggerChildren: 0.1,
        delayChildren: 0.2
      }
    }
  };

  const itemVariants = {
    closed: { opacity: 0, y: 20 },
    open: { opacity: 1, y: 0 }
  };

  if (!isOpen) return null;

  return (
    <motion.div
      initial="closed"
      animate="open"
      exit="closed"
      variants={menuVariants}
      className="fixed inset-0 z-[100] bg-white/95 backdrop-blur-2xl flex flex-col items-center justify-center p-8 overflow-hidden"
      role="dialog"
      aria-modal="true"
      aria-label="Menu de Navegação Mobile"
    >
      {/* Close Button */}
      <button 
        onClick={onClose}
        className="absolute top-8 right-8 p-3 rounded-full bg-black/5 text-neutral hover:bg-black/10 transition-colors outline-none focus-visible:ring-4 focus-visible:ring-primary/30"
        aria-label="Fechar Menu"
      >
        <X size={24} weight="bold" />
      </button>

      {/* Navigation Links */}
      <nav className="flex flex-col items-center gap-8">
        {navItems.map((item) => (
          <motion.button
            key={item.id}
            variants={itemVariants}
            onClick={() => {
              setActiveSection(item.id);
              onClose();
            }}
            className={`text-3xl font-semibold tracking-tight transition-all ${
              activeSection === item.id 
                ? 'text-primary' 
                : 'text-neutral/40 hover:text-neutral'
            }`}
          >
            {item.label}
          </motion.button>
        ))}
      </nav>

      {/* Footer Branding in Menu */}
      <motion.div 
        variants={itemVariants}
        className="absolute bottom-12 flex flex-col items-center gap-4 opacity-40"
      >
        <img src={logoUrl} alt="Zenith" className="h-8 w-8 grayscale" />
        <span className="text-[11px] font-mono tracking-widest uppercase">Performance & Cognição</span>
      </motion.div>
    </motion.div>
  );
};

export default MobileMenu;
