import React, { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { X, CaretDown, Palette } from '@phosphor-icons/react';

const MobileMenu = ({ isOpen, onClose, categories, activeSection, setActiveSection, labels }) => {
  const [expandedCat, setExpandedCat] = useState(null);
  const logoUrl = `${import.meta.env.BASE_URL}logo-only.svg`;

  const handleToggle = (id) => {
    setExpandedCat(expandedCat === id ? null : id);
  };

  const handleNavigate = (id) => {
    setActiveSection(id);
    onClose();
  };

  const menuVariants = {
    closed: { opacity: 0, x: '100%' },
    open: { 
      opacity: 1, 
      x: 0,
      transition: { duration: 0.4, ease: [0.22, 1, 0.36, 1] }
    }
  };

  return (
    <motion.div
      initial="closed"
      animate="open"
      exit="closed"
      variants={menuVariants}
      className="fixed inset-0 z-[100] bg-white flex flex-col p-6 overflow-y-auto"
    >
      {/* Header */}
      <div className="flex justify-between items-center mb-10">
        <img src={logoUrl} alt="Zenith" className="h-8 w-8" />
        <button 
          onClick={onClose}
          className="p-3 rounded-2xl bg-black/5 text-neutral"
        >
          <X size={24} weight="bold" />
        </button>
      </div>

      {/* Navigation */}
      <nav className="flex flex-col gap-2">
        {/* Home Direct Link */}
        <button
          onClick={() => handleNavigate('home')}
          className={`text-left py-4 text-3xl font-black tracking-tight ${activeSection === 'home' ? 'text-primary' : 'text-neutral/30'}`}
        >
          {labels.home}
        </button>

        {/* Categories Accordion */}
        {categories.map((cat) => (
          <div key={cat.id} className="flex flex-col border-b border-black/[0.03] py-2">
            <button
              onClick={() => handleToggle(cat.id)}
              className="flex items-center justify-between w-full py-4 text-left"
            >
              <span className={`text-2xl font-bold ${cat.items.some(i => i.id === activeSection) ? 'text-primary' : 'text-neutral/80'}`}>
                {cat.label}
              </span>
              <CaretDown size={20} className={`transition-transform duration-300 ${expandedCat === cat.id ? 'rotate-180' : ''}`} />
            </button>
            
            <AnimatePresence>
              {expandedCat === cat.id && (
                <motion.div
                  initial={{ height: 0, opacity: 0 }}
                  animate={{ height: 'auto', opacity: 1 }}
                  exit={{ height: 0, opacity: 0 }}
                  className="overflow-hidden flex flex-col gap-1 pl-4 pb-4"
                >
                  {cat.items.map((item) => (
                    <button
                      key={item.id}
                      onClick={() => handleNavigate(item.id)}
                      className={`text-left py-3 text-lg font-bold flex items-center justify-between ${activeSection === item.id ? 'text-primary' : 'text-neutral/40'}`}
                    >
                      <span>{item.label}</span>
                      {item.id === 'theme' && <Palette size={16} />}
                    </button>
                  ))}
                </motion.div>
              )}
            </AnimatePresence>
          </div>
        ))}
      </nav>

      <div className="mt-auto pt-10 flex flex-col items-center gap-2 opacity-20">
        <span className="text-[10px] font-black uppercase tracking-[0.4em]">Sovereign Web Core</span>
      </div>
    </motion.div>
  );
};

export default MobileMenu;
