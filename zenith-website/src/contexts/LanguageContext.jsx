import React, { createContext, useContext, useState, useEffect } from 'react';

const LanguageContext = createContext();

export const LANGUAGES = [
  { id: 'pt', label: 'Português', flag: 'BR' },
  { id: 'en', label: 'English', flag: 'US' },
  { id: 'ja', label: '日本語', flag: 'JP' },
  { id: 'es', label: 'Español', flag: 'ES' }
];

export const LanguageProvider = ({ children }) => {
  const [lang, setLang] = useState(() => {
    return localStorage.getItem('zenith_lang') || 'pt';
  });

  useEffect(() => {
    localStorage.setItem('zenith_lang', lang);
    document.documentElement.lang = lang;
  }, [lang]);

  return (
    <LanguageContext.Provider value={{ lang, setLang, languages: LANGUAGES }}>
      {children}
    </LanguageContext.Provider>
  );
};

export const useLanguage = () => {
  const context = useContext(LanguageContext);
  if (!context) {
    throw new Error('useLanguage must be used within a LanguageProvider');
  }
  return context;
};
