import React from 'react';
import { motion } from 'framer-motion';
import { GameController, Brain, Lightning, ArrowRight, GithubLogo } from '@phosphor-icons/react';

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
              <motion.button 
                whileHover={{ scale: 1.02, y: -2 }}
                whileTap={{ scale: 0.98 }}
                data-z-id="home-hero-btn-docs" 
                onClick={() => onNavigate('handbook')}
                className="group flex items-center justify-center gap-2 bg-[#111111] text-white px-7 py-3.5 rounded-[8px] text-[15px] font-medium tracking-wide transition-all hover:bg-[#333333] hover:shadow-xl hover:shadow-black/10 w-full sm:w-auto"
              >
                Ler Documentação
                <ArrowRight data-z-id="home-hero-btn-docs-icon" weight="bold" className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
              </motion.button>
              <motion.button 
                whileHover={{ scale: 1.02, y: -2 }}
                whileTap={{ scale: 0.98 }}
                data-z-id="home-hero-btn-github" 
                className="group flex items-center justify-center gap-2 bg-white/50 backdrop-blur-sm text-[#111111] border border-[#EAEAEA] px-7 py-3.5 rounded-[8px] text-[15px] font-medium tracking-wide transition-all hover:bg-white hover:border-[#D1D1D1] hover:shadow-lg hover:shadow-black/5 w-full sm:w-auto"
              >
                <GithubLogo data-z-id="home-hero-btn-github-icon" weight="fill" className="w-5 h-5" />
                GitHub
              </motion.button>
            </motion.div>
          </div>

          {/* Right Column: Abstract/Code Representation */}
          <motion.div data-z-id="home-hero-illustration-wrapper" variants={itemVariants} className="flex-1 w-full relative hidden lg:block">
            <div data-z-id="home-hero-illustration-container" className="w-full aspect-square max-w-[480px] ml-auto relative">
              <motion.div 
                whileHover={{ y: -8, transition: { duration: 0.4, ease: "easeOut" } }}
                data-z-id="home-hero-code-window" 
                className="absolute inset-0 border border-white/20 rounded-[20px] bg-white/40 backdrop-blur-xl flex flex-col overflow-hidden shadow-[0_30px_60px_-15px_rgba(0,0,0,0.05)] border-t-white/40"
              >
                <div data-z-id="home-hero-code-header" className="h-12 border-b border-black/5 flex items-center px-5 gap-2 bg-white/20">
                  <div data-z-id="home-hero-code-dot-1" className="w-2.5 h-2.5 rounded-full bg-[#EAEAEA]"></div>
                  <div data-z-id="home-hero-code-dot-2" className="w-2.5 h-2.5 rounded-full bg-[#EAEAEA]"></div>
                  <div data-z-id="home-hero-code-dot-3" className="w-2.5 h-2.5 rounded-full bg-[#EAEAEA]"></div>
                </div>
                <motion.div 
                  variants={codeContainerVariants}
                  data-z-id="home-hero-code-content" 
                  className="p-8 font-mono text-[13px] sm:text-[14px] leading-[2.2] text-[#787774] overflow-x-auto whitespace-nowrap"
                >
                  <motion.div variants={codeLineVariants}><span className="text-[#D33969] font-semibold">import</span> <span className="text-[#6F42C1]">std.ui</span></motion.div><br/>
                  <motion.div variants={codeLineVariants}><span className="text-[#D33969] font-semibold">func</span> <span className="text-[#111111] font-bold">App</span>()</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;<span className="text-[#D33969] font-semibold">state</span> count: <span className="text-[#0071E3]">int</span> = <span className="text-[#D19A66]">0</span></motion.div><br/>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;<span className="text-[#D33969] font-semibold">func</span> <span className="text-[#111111] font-bold">increment</span>()</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;&nbsp;&nbsp;count = count + <span className="text-[#D19A66]">1</span></motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;<span className="text-[#D33969] font-semibold">end</span></motion.div><br/>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;<span className="text-[#D33969] font-semibold">return</span> <span className="text-[#6F42C1]">ui.Window</span> &#123;</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;&nbsp;&nbsp;<span className="text-[#E36209]">title</span>: <span className="text-[#22863A]">"Zenith"</span>,</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;&nbsp;&nbsp;<span className="text-[#E36209]">body</span>: <span className="text-[#6F42C1]">ui.Button</span> &#123;</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span className="text-[#E36209]">text</span>: <span className="text-[#22863A]">"Cliques: &#123;count&#125;"</span>,</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span className="text-[#E36209]">on_click</span>: increment</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;&nbsp;&nbsp;&#125;</motion.div>
                  <motion.div variants={codeLineVariants}>&nbsp;&nbsp;&#125;</motion.div>
                  <motion.div variants={codeLineVariants}><span className="text-[#D33969] font-semibold">end</span></motion.div>
                </motion.div>
              </motion.div>
              {/* Subtle decorative geometric accents */}
              <div data-z-id="home-hero-decor-1" className="absolute -bottom-8 -left-8 w-40 h-40 border border-[#EAEAEA] rounded-full opacity-50 z-[-1]"></div>
              <div data-z-id="home-hero-decor-2" className="absolute -top-4 -right-4 w-24 h-24 bg-primary/5 rounded-full blur-2xl z-[-1]"></div>
            </div>
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