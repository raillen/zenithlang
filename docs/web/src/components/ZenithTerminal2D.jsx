import React, { useState, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';

const CODE_SAMPLES = [
  {
    title: "main.zt",
    lines: [
      "import std.io",
      "",
      "func main() {",
      "  print(\"Hello Zenith\")",
      "  let x = 10",
      "  if x > 5 {",
      "    print(\"Performance!\")",
      "  }",
      "}"
    ]
  },
  {
    title: "data.zt",
    lines: [
      "struct User {",
      "  id: int,",
      "  name: str,",
      "  active: bool",
      "}",
      "",
      "impl User {",
      "  func name() -> str {",
      "    return self.name",
      "  }",
      "}"
    ]
  },
  {
    title: "ui.zt",
    lines: [
      "import std.ui",
      "",
      "func view() {",
      "  return ui.Window {",
      "    title: \"Zenith Hub\",",
      "    body: ui.VStack {",
      "      ui.Text(\"Premium UX\"),",
      "      ui.Button(\"Explore\")",
      "    }",
      "  }",
      "}"
    ]
  }
];

const ZenithTerminal2D = () => {
  const [currentSample, setCurrentSample] = useState(0);
  const [typingIndex, setTypingIndex] = useState(0);
  const [lineIndex, setLineIndex] = useState(0);
  const [isDeleting, setIsDeleting] = useState(false);

  useEffect(() => {
    const sample = CODE_SAMPLES[currentSample].lines;
    const currentLine = sample[lineIndex];

    const timer = setTimeout(() => {
      if (!isDeleting) {
        if (lineIndex < sample.length) {
          if (typingIndex < currentLine.length) {
            setTypingIndex(prev => prev + 1);
          } else {
            setLineIndex(prev => prev + 1);
            setTypingIndex(0);
          }
        } else {
          setTimeout(() => setIsDeleting(true), 4000);
        }
      } else {
        if (lineIndex > 0 || typingIndex > 0) {
          if (typingIndex > 0) {
            setTypingIndex(prev => prev - 1);
          } else {
            setLineIndex(prev => prev - 1);
            setTypingIndex(sample[lineIndex - 1]?.length || 0);
          }
        } else {
          setIsDeleting(false);
          setCurrentSample((prev) => (prev + 1) % CODE_SAMPLES.length);
        }
      }
    }, isDeleting ? 15 : 40);

    return () => clearTimeout(timer);
  }, [typingIndex, lineIndex, isDeleting, currentSample]);

  return (
    <motion.div 
      initial={{ opacity: 0, scale: 0.98 }}
      animate={{ opacity: 1, scale: 1 }}
      className="w-full h-full glass-dark p-0 flex flex-col rounded-[24px] shadow-premium border border-black/5 overflow-hidden font-mono text-[14px] absolute inset-0"
    >
      {/* Terminal Bar */}
      <div className="flex items-center justify-between px-5 h-11 bg-black/5 border-b border-black/5">
        <div className="flex gap-2">
          <div className="w-3 h-3 rounded-full bg-red-400/40" />
          <div className="w-3 h-3 rounded-full bg-amber-400/40" />
          <div className="w-3 h-3 rounded-full bg-emerald-400/40" />
        </div>
        <div className="text-[11px] font-bold tracking-widest text-neutral/40 flex items-center gap-2">
          <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse" />
          {CODE_SAMPLES[currentSample].title}
        </div>
        <div className="w-10" /> {/* Spacer */}
      </div>

      {/* Terminal Viewport */}
      <div className="flex-1 p-8 text-emerald-600 relative overflow-hidden bg-[#FAFAFA]/50 cursor-text">
        <div className="relative z-10">
          {CODE_SAMPLES[currentSample].lines.slice(0, lineIndex).map((line, i) => (
            <motion.div 
              initial={{ opacity: 0, x: -5 }}
              animate={{ opacity: 1, x: 0 }}
              key={i} 
              className="mb-1.5"
            >
              <span className="text-neutral/20 mr-4 select-none">{i + 1}</span>
              {line}
            </motion.div>
          ))}
          
          <div className="inline-flex items-center">
            <span className="text-neutral/20 mr-4 select-none">{lineIndex + 1}</span>
            <span className="relative">
              {CODE_SAMPLES[currentSample].lines[lineIndex]?.substring(0, typingIndex)}
              <motion.span
                animate={{ opacity: [1, 0] }}
                transition={{ repeat: Infinity, duration: 0.8 }}
                className="inline-block w-2.5 h-5 bg-emerald-500/80 ml-1.5 align-middle"
              />
            </span>
          </div>
        </div>

        {/* Scanlines Efeito Retro Premium */}
        <div className="absolute inset-0 pointer-events-none opacity-[0.03] bg-[linear-gradient(rgba(18,16,16,0)_50%,rgba(0,0,0,0.25)_50%),linear-gradient(90deg,rgba(255,0,0,0.06),rgba(0,255,0,0.02),rgba(0,0,255,0.06))] bg-[length:100%_4px,4px_100%]" />
        
        {/* Glow de fundo */}
        <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-3/4 h-3/4 bg-emerald-500/5 blur-[100px] pointer-events-none rounded-full" />
      </div>

      {/* Footer Info */}
      <div className="h-10 px-5 flex items-center justify-between text-[11px] text-neutral/30 border-t border-black/5 bg-white/50 backdrop-blur-sm">
        <div className="flex gap-4">
          <span>UTF-8</span>
          <span>Zenith v0.2</span>
        </div>
        <div>Ln {lineIndex + 1}, Col {typingIndex + 1}</div>
      </div>
    </motion.div>
  );
};

export default ZenithTerminal2D;
