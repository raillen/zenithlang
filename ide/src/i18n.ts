import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';

const resources = {
  en: {
    translation: {
      "project": "Project",
      "search_placeholder": "Search files or commands...",
      "ready_status": "Ready",
      "no_file_selected": "No file selected",
      "loading": "Loading...",
      "switch_theme": "Switch Theme",
      "font_dyslexic": "Dyslexic Font"
    }
  },
  pt: {
    translation: {
      "project": "Projeto",
      "search_placeholder": "Buscar arquivos ou comandos...",
      "ready_status": "Pronto",
      "no_file_selected": "Nenhum arquivo selecionado",
      "loading": "Carregando...",
      "switch_theme": "Mudar Tema",
      "font_dyslexic": "Fonte Dislexia"
    }
  }
};

i18n
  .use(initReactI18next)
  .init({
    resources,
    lng: "en", 
    fallbackLng: "en",
    interpolation: {
      escapeValue: false
    }
  });

export default i18n;
