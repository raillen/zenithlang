import React from 'react';
import { motion } from 'framer-motion';
import { GameController, Brain, Lightning, ArrowRight } from '@phosphor-icons/react';
import MagneticWrapper from '../components/MagneticWrapper';
import ZenithTerminal2D from '../components/ZenithTerminal2D';
import { useLanguage } from '../contexts/LanguageContext';
import { UI_STRINGS } from '../data/translations';

const Home = ({ onNavigate }) => {
  const { lang } = useLanguage();
  const t = UI_STRINGS[lang].home;
  
  const logoUrl = `${import.meta.env.BASE_URL}logo-only.svg`;
  const containerVariants = {
    hidden: { opacity: 0 },
    visible: {
      opacity: 1,
      transition: { staggerChildren: 0.1, delayChildren: 0.1 },
    },
  };

  const itemVariants = {
    hidden: { opacity: 0, y: 24 },
    visible: {
      opacity: 1,
      y: 0,
      transition: { type: 'spring', stiffness: 100, damping: 20 },
    },
  };

  return (
    <motion.div
      data-z-id="home-page"
      initial="hidden"
      animate="visible"
      exit={{ opacity: 0 }}
      className="page-home flex min-h-[calc(100dvh-140px)] flex-col items-center justify-center overflow-hidden py-16 md:py-24"
    >
      <section
        data-z-id="home-hero-section"
        className="z-10 mx-auto flex w-full max-w-[1200px] flex-col gap-24 px-6"
      >
        <motion.div
          data-z-id="home-hero-container"
          variants={containerVariants}
          className="flex flex-col items-center justify-between gap-12 md:flex-row md:items-start md:gap-24"
        >
          <div
            data-z-id="home-hero-content"
            className="flex w-full max-w-[600px] flex-1 flex-col items-center gap-8 text-center md:items-start md:text-left"
          >
            <motion.div
              data-z-id="home-hero-badge"
              variants={itemVariants}
              className="mb-2 flex items-center gap-4"
            >
              <img
                data-z-id="home-hero-logo"
                src={logoUrl}
                alt="Zenith Logo"
                className="h-auto w-8 grayscale opacity-60 md:w-10"
              />
              <div
                data-z-id="home-hero-divider"
                className="h-[1px] w-6 bg-black/10 md:w-8"
              />
              <span className="text-[10px] font-mono font-semibold uppercase tracking-[0.2em] text-neutral/40 md:text-[11px]">
                v0.3.0 Ascension Final
              </span>
            </motion.div>

            <motion.h1
              data-z-id="home-hero-title"
              variants={itemVariants}
              className="-mt-4 text-4xl font-semibold leading-[1.05] tracking-[-0.02em] text-[#111111] sm:text-5xl md:text-[5.5rem] whitespace-pre-line"
            >
              {t.title}
            </motion.h1>

            <motion.p
              data-z-id="home-hero-description"
              variants={itemVariants}
              className="-mt-4 max-w-[42ch] text-[18px] font-serif leading-[1.6] text-[#787774] md:-mt-6 md:text-[22px]"
            >
              {t.description}
            </motion.p>

            <motion.div
              data-z-id="home-hero-actions"
              variants={itemVariants}
              className="-mt-2 flex w-full flex-col items-center gap-4 sm:flex-row"
            >
              <MagneticWrapper strength={0.4} className="w-full sm:w-auto">
                <motion.button
                  whileHover={{ scale: 1.02 }}
                  whileTap={{ scale: 0.98 }}
                  data-z-id="home-hero-btn-docs"
                  onClick={() => onNavigate('start')}
                  className="group flex w-full items-center justify-center gap-2 rounded-[8px] bg-[#111111] px-7 py-3.5 text-[15px] font-medium tracking-wide text-white transition-all hover:bg-[#333333] hover:shadow-xl hover:shadow-black/10"
                >
                  {t.readVision}
                  <ArrowRight
                    data-z-id="home-hero-btn-docs-icon"
                    weight="bold"
                    className="h-4 w-4 transition-transform group-hover:translate-x-1"
                  />
                </motion.button>
              </MagneticWrapper>

              <MagneticWrapper strength={0.4} className="w-full sm:w-auto">
                <motion.button
                  whileHover={{ scale: 1.02 }}
                  whileTap={{ scale: 0.98 }}
                  data-z-id="home-hero-btn-compilers"
                  onClick={() => onNavigate('compiler')}
                  className="group flex w-full items-center justify-center gap-2 rounded-[8px] border border-[#EAEAEA] bg-white/50 px-7 py-3.5 text-[15px] font-medium tracking-wide text-[#111111] backdrop-blur-sm transition-all hover:border-[#D1D1D1] hover:bg-white hover:shadow-lg hover:shadow-black/5"
                >
                  {t.viewCompilers}
                  <ArrowRight
                    data-z-id="home-hero-btn-compilers-icon"
                    weight="bold"
                    className="h-4 w-4 transition-transform group-hover:translate-x-1"
                  />
                </motion.button>
              </MagneticWrapper>
            </motion.div>
          </div>

          <motion.div
            data-z-id="home-hero-illustration-wrapper"
            variants={itemVariants}
            className="relative flex min-h-[400px] w-full flex-1 items-center justify-center overflow-hidden rounded-[24px] border border-black/5 bg-white/5 shadow-premium md:min-h-[480px] md:rounded-[32px]"
          >
            <div className="absolute inset-0 bg-gradient-to-tr from-primary/5 to-transparent opacity-30" />
            <ZenithTerminal2D />
          </motion.div>
        </motion.div>

        <motion.div
          data-z-id="home-features-grid"
          variants={containerVariants}
          className="grid grid-cols-1 gap-5 md:grid-cols-12"
        >
          <FeatureCard
            data-z-id="home-feature-performance"
            className="md:col-span-7"
            title={t.feature1Title}
            description={t.feature1Desc}
            icon={<GameController weight="fill" className="h-5 w-5" />}
          />

          <FeatureCard
            data-z-id="home-feature-adhd"
            className="md:col-span-5"
            title={t.feature2Title}
            description={t.feature2Desc}
            icon={<Brain weight="fill" className="h-5 w-5" />}
          />

          <FeatureCard
            data-z-id="home-feature-reactivity"
            className="md:col-span-12"
            title={t.feature3Title}
            description={t.feature3Desc}
            icon={<Lightning weight="fill" className="h-5 w-5" />}
            horizontal
          />
        </motion.div>
      </section>
    </motion.div>
  );
};

const FeatureCard = ({ title, description, icon, className = '', horizontal = false, ...props }) => (
  <motion.div
    data-z-id={props['data-z-id'] || 'feature-card'}
    variants={{
      hidden: { opacity: 0, y: 20 },
      visible: {
        opacity: 1,
        y: 0,
        transition: { type: 'spring', stiffness: 100, damping: 20 },
      },
    }}
    className={`group relative overflow-hidden rounded-[16px] border border-[#EAEAEA] bg-white p-8 transition-all duration-500 hover:border-[#D1D1D1] hover:shadow-[0_20px_40px_-15px_rgba(0,0,0,0.04)] md:p-10 ${className} ${horizontal ? 'flex flex-col items-start gap-6 md:flex-row md:items-center md:gap-10' : 'flex flex-col gap-6'}`}
    role="article"
    aria-labelledby={`${props['data-z-id'] || 'feature-card'}-title`}
  >
    <div
      data-z-id={`${props['data-z-id'] || 'feature-card'}-icon-container`}
      className="flex h-12 w-12 shrink-0 items-center justify-center rounded-[10px] border border-[#EAEAEA] bg-[#FBFBFA] text-[#111111] transition-all duration-500 group-hover:scale-105 group-hover:border-[#111111] group-hover:bg-[#111111] group-hover:text-white"
      aria-hidden="true"
    >
      {icon}
    </div>
    <div
      data-z-id={`${props['data-z-id'] || 'feature-card'}-content`}
      className="flex flex-1 flex-col gap-2.5"
    >
      <h2
        data-z-id={`${props['data-z-id'] || 'feature-card'}-title`}
        id={`${props['data-z-id'] || 'feature-card'}-title`}
        className="text-[1.25rem] font-semibold leading-tight tracking-tight text-[#111111]"
      >
        {title}
      </h2>
      <p
        data-z-id={`${props['data-z-id'] || 'feature-card'}-description`}
        className="max-w-[55ch] text-[15px] leading-relaxed text-[#787774]"
      >
        {description}
      </p>
    </div>
  </motion.div>
);

export default Home;