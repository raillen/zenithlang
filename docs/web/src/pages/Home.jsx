import React from 'react';
import { motion } from 'framer-motion';
import { GameController, Brain, Lightning, ArrowRight, GithubLogo } from '@phosphor-icons/react';
import MagneticWrapper from '../components/MagneticWrapper';
import ZenithTerminal2D from '../components/ZenithTerminal2D';

const Home = ({ onNavigate }) => {
  // Motion configurations
  const containerVariants = {
    hidden: { opacity: 0 },
    visible: {
      opacity: 1,
      transition: { staggerChildren: 0.1, delayChildren: 0.1 }
    }
  };

  const itemVariants = {
    hidden: { opacity: 0, y: 24 },
    visible: { 
      opacity: 1, 
      y: 0, 
      transition: { type: "spring", stiffness: 100, damping: 20 } 
    }
  };

  const codeContainerVariants = {
    hidden: { opacity: 0 },
    visible: {
      opacity: 1,
      transition: {
        staggerChildren: 0.05,
        delayChildren: 0.8
      }
    }
  };

  const codeLineVariants = {
    hidden: { opacity: 0, x: -10 },
    visible: { 
      opacity: 1, 
      x: 0,
      transition: { duration: 0.3 }
    }
  };

  return (
    <motion.div 
      data-z-id="home-page"
      initial="hidden"
      animate="visible"
      exit={{ opacity: 0 }}
      className="page-home flex flex-col items-center min-h-[calc(100dvh-140px)] justify-center py-16 md:py-24 overflow-hidden"
    >
      <section data-z-id="home-hero-section" className="w-full px-6 max-w-[1200px] mx-auto flex flex-col gap-24 z-10">
        
        {/* HERO SECTION - Asymmetric & Editorial */}
        <motion.div 
          data-z-id="home-hero-container"
          variants={containerVariants}
          className="flex flex-col md:flex-row items-center md:items-start justify-between gap-16 md:gap-24"
        >
          {/* Left Column: Typography */}
          <div data-z-id="home-hero-content" className="flex-1 flex flex-col items-start gap-8 w-full max-w-[600px]">
            <motion.div data-z-id="home-hero-badge" variants={itemVariants} className="flex items-center gap-4 mb-2">
              <img data-z-id="home-hero-logo" src="/logo-only.svg" alt="Zenith Logo" className="w-10 h-auto grayscale opacity-60" />
              <div data-z-id="home-hero-divider" className="h-[1px] w-8 bg-black/10"></div>
              <span data-z-id="home-hero-version" className="text-[11px] font-mono font-semibold tracking-[0.2em] text-neutral/40 uppercase">v0.2 Milestone</span>
            </motion.div>

            <motion.h1
              data-z-id="home-hero-title"
              variants={itemVariants}
              className="-mt-4 text-5xl md:text-[5.5rem] font-semibold tracking-[-0.02em] text-[#111111] leading-[1.05]"
            >              Performance.<br />
              Cognição.<br />
              <span data-z-id="home-hero-title-highlight" className="font-serif italic text-primary pr-4 font-normal">Zenith.</span>
            </motion.h1>

            <motion.p 
              data-z-id="home-hero-description"
              variants={itemVariants}
              className="-mt-6 text-[20px] md:text-[22px] text-[#787774] font-serif leading-[1.6] max-w-[42ch]"
            >
              A linguagem de programação projetada para o cérebro moderno. Otimizada para jogos, automação e mínimo esforço cognitivo.
            </motion.p>
            
            <motion.div data-z-id="home-hero-actions" variants={itemVariants} className="flex flex-wrap items-center gap-4 -mt-2 w-full">
              <MagneticWrapper strength={0.4}>
                <motion.button 
                  whileHover={{ scale: 1.02 }}
                  whileTap={{ scale: 0.98 }}
                  data-z-id="home-hero-btn-docs" 
                  onClick={() => onNavigate('handbook')}
                  className="group flex items-center justify-center gap-2 bg-[#111111] text-white px-7 py-3.5 rounded-[8px] text-[15px] font-medium tracking-wide transition-all hover:bg-[#333333] hover:shadow-xl hover:shadow-black/10 w-full sm:w-auto"
                >
                  Ler Documentação
                  <ArrowRight data-z-id="home-hero-btn-docs-icon" weight="bold" className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
                </motion.button>
              </MagneticWrapper>
              <MagneticWrapper strength={0.4}>
                <motion.button 
                  whileHover={{ scale: 1.02 }}
                  whileTap={{ scale: 0.98 }}
                  data-z-id="home-hero-btn-github" 
                  className="group flex items-center justify-center gap-2 bg-white/50 backdrop-blur-sm text-[#111111] border border-[#EAEAEA] px-7 py-3.5 rounded-[8px] text-[15px] font-medium tracking-wide transition-all hover:bg-white hover:border-[#D1D1D1] hover:shadow-lg hover:shadow-black/5 w-full sm:w-auto"
                >
                  <GithubLogo data-z-id="home-hero-btn-github-icon" weight="fill" className="w-5 h-5" />
                  GitHub
                </motion.button>
              </MagneticWrapper>
            </motion.div>
          </div>

          {/* Right Column: Interactive 3D Representation */}
          <motion.div 
            data-z-id="home-hero-illustration-wrapper" 
            variants={itemVariants} 
            className="flex-1 w-full relative hidden lg:flex items-center justify-center bg-white/5 rounded-[32px] border border-black/5 shadow-premium overflow-hidden min-h-[480px]"
          >
            <div className="absolute inset-0 bg-gradient-to-tr from-primary/5 to-transparent opacity-30" />
            <ZenithTerminal2D />
          </motion.div>
        </motion.div>

        {/* FEATURES - Bento Grid 2.0 (Editorial & Asymmetric) */}
        <motion.div data-z-id="home-features-grid" variants={containerVariants} className="grid grid-cols-1 md:grid-cols-12 gap-5">
          
          <FeatureCard 
            data-z-id="home-feature-performance"
            className="md:col-span-7"
            title="Alta Performance (Lua)" 
            description="Compilação 'zero-cost' focada em engines como Love2D e Solar2D, entregando framerates massivos sem comprometer a elegância da sintaxe."
            icon={<GameController weight="fill" className="w-5 h-5" />}
          />
          
          <FeatureCard 
            data-z-id="home-feature-adhd"
            className="md:col-span-5"
            title="TDAH Friendly" 
            description="Remoção rigorosa de ruído visual. Sintaxe desenhada para reduzir carga cognitiva e fadiga de decisão."
            icon={<Brain weight="fill" className="w-5 h-5" />}
          />
          
          <FeatureCard 
            data-z-id="home-feature-reactivity"
            className="md:col-span-12"
            title="Reatividade Pura & Integrada" 
            description="Um sistema de estado nativo que entende as mutações da sua aplicação de forma reativa, eliminando toneladas de código boilerplate e o gerenciamento manual de eventos."
            icon={<Lightning weight="fill" className="w-5 h-5" />}
            horizontal
          />
          
        </motion.div>
      </section>
    </motion.div>
  );
};

const FeatureCard = ({ title, description, icon, className = "", horizontal = false, ...props }) => (
  <motion.div 
    data-z-id={props['data-z-id'] || 'feature-card'}
    variants={{
      hidden: { opacity: 0, y: 20 },
      visible: { opacity: 1, y: 0, transition: { type: "spring", stiffness: 100, damping: 20 } }
    }}
    className={`group relative bg-white border border-[#EAEAEA] rounded-[16px] p-8 md:p-10 overflow-hidden hover:shadow-[0_20px_40px_-15px_rgba(0,0,0,0.04)] hover:border-[#D1D1D1] transition-all duration-500 ${className} ${horizontal ? 'flex flex-col md:flex-row items-start md:items-center gap-6 md:gap-10' : 'flex flex-col gap-6'}`}
  >
    <div data-z-id={`${props['data-z-id'] || 'feature-card'}-icon-container`} className="flex items-center justify-center w-12 h-12 rounded-[10px] bg-[#FBFBFA] border border-[#EAEAEA] text-[#111111] group-hover:bg-[#111111] group-hover:text-white group-hover:border-[#111111] group-hover:scale-105 transition-all duration-500 shrink-0">
      {icon}
    </div>
    <div data-z-id={`${props['data-z-id'] || 'feature-card'}-content`} className="flex flex-col gap-2.5 flex-1">
      <h3 data-z-id={`${props['data-z-id'] || 'feature-card'}-title`} className="text-[1.25rem] font-semibold tracking-tight text-[#111111] leading-tight">
        {title}
      </h3>
      <p data-z-id={`${props['data-z-id'] || 'feature-card'}-description`} className="text-[15px] text-[#787774] font-normal leading-relaxed max-w-[55ch]">
        {description}
      </p>
    </div>
  </motion.div>
);

export default Home;