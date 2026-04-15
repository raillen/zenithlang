import React, { useRef, useState, useEffect } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { 
  PerspectiveCamera, 
  PresentationControls,
  Environment,
  Html,
  Float,
  ContactShadows
} from '@react-three/drei';
import * as THREE from 'three';

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
      "    title: \"Zenith Panel\",",
      "    body: ui.VStack {",
      "      ui.Text(\"Ready\"),",
      "      ui.Button(\"Run\")",
      "    }",
      "  }",
      "}"
    ]
  }
];

const TerminalScreen = () => {
  const [currentSample, setCurrentSample] = useState(0);
  const [displayText, setDisplayText] = useState([]);
  const [typingIndex, setTypingIndex] = useState(0);
  const [lineIndex, setLineIndex] = useState(0);
  const [isDeleting, setIsDeleting] = useState(false);

  useEffect(() => {
    const sample = CODE_SAMPLES[currentSample].lines;
    const currentLine = sample[lineIndex];

    const timer = setTimeout(() => {
      if (!isDeleting) {
        // Typing
        if (lineIndex < sample.length) {
          if (typingIndex < currentLine.length) {
            setTypingIndex(prev => prev + 1);
          } else {
            // Line finished
            setLineIndex(prev => prev + 1);
            setTypingIndex(0);
          }
        } else {
          // All code finished, wait then delete
          setTimeout(() => setIsDeleting(true), 3000);
        }
      } else {
        // Deleting
        if (lineIndex > 0 || typingIndex > 0) {
          if (typingIndex > 0) {
            setTypingIndex(prev => prev - 1);
          } else {
            setLineIndex(prev => prev - 1);
            setTypingIndex(sample[lineIndex - 1]?.length || 0);
          }
        } else {
          // Finish deleting, move to next sample
          setIsDeleting(false);
          setCurrentSample((prev) => (prev + 1) % CODE_SAMPLES.length);
        }
      }
    }, isDeleting ? 20 : 50);

    return () => clearTimeout(timer);
  }, [typingIndex, lineIndex, isDeleting, currentSample]);

  return (
    <div className="w-[400px] h-[300px] bg-black/80 backdrop-blur-xl border border-white/10 rounded-xl overflow-hidden shadow-2xl flex flex-col font-mono text-[13px] select-none">
      {/* Header */}
      <div className="h-8 bg-white/5 border-b border-white/10 flex items-center px-4 justify-between">
        <div className="flex gap-1.5">
          <div className="w-2.5 h-2.5 rounded-full bg-red-500/50" />
          <div className="w-2.5 h-2.5 rounded-full bg-amber-500/50" />
          <div className="w-2.5 h-2.5 rounded-full bg-emerald-500/50" />
        </div>
        <span className="text-white/30 text-[10px] uppercase tracking-widest font-bold">
          {CODE_SAMPLES[currentSample].title}
        </span>
      </div>

      {/* Content */}
      <div className="flex-1 p-6 text-emerald-400 overflow-hidden relative">
        <div className="absolute inset-0 bg-gradient-to-t from-emerald-500/5 to-transparent pointer-events-none" />
        
        {/* Render fully typed lines */}
        {CODE_SAMPLES[currentSample].lines.slice(0, lineIndex).map((line, i) => (
          <div key={i} className="mb-1">{line}</div>
        ))}
        
        {/* Render current typing line */}
        <div className="inline-block relative">
          {CODE_SAMPLES[currentSample].lines[lineIndex]?.substring(0, typingIndex)}
          <span className="w-2 h-4 bg-emerald-400/80 inline-block ml-0.5 animate-pulse vertical-middle mb-[-3px]" />
        </div>
        
        {/* Scanline Effect */}
        <div className="absolute inset-0 pointer-events-none opacity-10 pointer-events-none bg-[linear-gradient(rgba(18,16,16,0)_50%,rgba(0,0,0,0.25)_50%),linear-gradient(90deg,rgba(255,0,0,0.06),rgba(0,255,0,0.02),rgba(0,0,255,0.06))] bg-[length:100%_2px,3px_100%]" />
      </div>
    </div>
  );
};

const ZenithTerminal3D = () => {
  return (
    <div className="w-full h-full min-h-[500px]">
      <Canvas shadows dpr={[1, 2]} camera={{ position: [0, 0, 10], fov: 35 }}>
        <PerspectiveCamera makeDefault position={[0, 0, 8]} fov={40} />
        <Environment preset="city" />
        
        <ambientLight intensity={0.5} />
        <spotLight position={[10, 10, 10]} angle={0.15} penumbra={1} intensity={1} castShadow />

        <PresentationControls
          global
          config={{ mass: 2, tension: 500 }}
          snap={{ mass: 4, tension: 1500 }}
          rotation={[0, -0.3, 0]}
          polar={[-Math.PI / 4, Math.PI / 4]}
          azimuth={[-Math.PI / 3, Math.PI / 3]}
        >
          <Float speed={1.5} rotationIntensity={0.5} floatIntensity={1}>
            <Html transform distanceFactor={5} position={[0, 0, 0]} occlude="blending">
              <TerminalScreen />
            </Html>
          </Float>
        </PresentationControls>
        
        <ContactShadows position={[0, -3.5, 0]} opacity={0.2} scale={15} blur={3} far={4} />
      </Canvas>
    </div>
  );
};

export default ZenithTerminal3D;
