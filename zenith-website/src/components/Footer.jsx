import React from 'react';

const Footer = ({ onNavigate }) => {
  const logoUrl = `${import.meta.env.BASE_URL}logo-only.svg`;
  return (
    <footer data-z-id="app-footer" className="site-footer py-8 px-6 mt-10 border-t border-black/5 text-center">
      <div data-z-id="app-footer-container" className="max-w-5xl mx-auto flex flex-col md:flex-row items-center justify-between gap-4">
        <div data-z-id="app-footer-branding" className="flex items-center gap-3 text-neutral/50 font-medium text-sm">
           <img data-z-id="app-footer-logo" src={logoUrl} alt="Zenith" className="h-5 w-5 opacity-50 grayscale" />
           <span data-z-id="app-footer-copyright" className="flex items-center gap-1.5">
             © 2026 Zenith. Documentação oficial e site editorial por
             <a 
               href="https://www.instagram.com/raillen.santos" 
               target="_blank" 
               rel="noopener noreferrer"
               className="font-bold hover:text-primary transition-colors duration-300"
             >
               Raillen Santos
             </a>
           </span>
        </div>
        <nav data-z-id="app-footer-nav" className="flex gap-6 text-sm font-medium text-neutral/80">
          <button data-z-id="app-footer-link-vision" type="button" onClick={() => onNavigate('start')} className="hover:text-neutral transition-colors">Visão</button>
          <button data-z-id="app-footer-link-compilers" type="button" onClick={() => onNavigate('compiler')} className="hover:text-neutral transition-colors">Compiladores</button>
          <button data-z-id="app-footer-link-reference" type="button" onClick={() => onNavigate('reference')} className="hover:text-neutral transition-colors">Referência</button>
        </nav>
      </div>
    </footer>
  );
};

export default Footer;
