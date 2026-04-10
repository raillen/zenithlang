import React, { useEffect, useState } from 'react';
import { motion, useSpring, useMotionValue } from 'framer-motion';

const CustomCursor = () => {
  const [cursorVariant, setCursorVariant] = useState("default");
  
  const cursorX = useMotionValue(-100);
  const cursorY = useMotionValue(-100);
  
  const springConfig = { damping: 20, stiffness: 400, mass: 0.5 };
  const cursorXSpring = useSpring(cursorX, springConfig);
  const cursorYSpring = useSpring(cursorY, springConfig);

  useEffect(() => {
    const moveCursor = (e) => {
      cursorX.set(e.clientX);
      cursorY.set(e.clientY);
      
      const target = e.target;
      const computedStyle = window.getComputedStyle(target);
      
      const isClickable = computedStyle.cursor === 'pointer' || target.closest('button') || target.closest('a');
      const isTextArea = computedStyle.cursor === 'text' || target.closest('.cursor-text') || target.closest('pre') || target.closest('code') || target.closest('.prose p');
      
      if (isClickable) {
        setCursorVariant("pointer");
      } else if (isTextArea) {
        setCursorVariant("text");
      } else {
        setCursorVariant("default");
      }
    };

    window.addEventListener('mousemove', moveCursor);
    return () => window.removeEventListener('mousemove', moveCursor);
  }, []);

  const variants = {
    default: {
      width: 24,
      height: 24,
      borderRadius: "50%",
      backgroundColor: "rgba(0, 113, 227, 0)",
      border: "2px solid rgba(0, 113, 227, 0.5)",
      x: -12,
      y: -12,
    },
    pointer: {
      width: 48,
      height: 48,
      borderRadius: "50%",
      backgroundColor: "rgba(0, 113, 227, 0.1)",
      border: "1px solid rgba(0, 113, 227, 0.3)",
      x: -24,
      y: -24,
    },
    text: {
      width: 2,
      height: 24,
      borderRadius: "0%",
      backgroundColor: "rgba(0, 113, 227, 1)",
      border: "0px solid transparent",
      x: -1,
      y: -12,
    }
  };

  return (
    <motion.div
      className="fixed top-0 left-0 pointer-events-none z-[9999] mix-blend-difference hidden md:block overflow-hidden"
      variants={variants}
      animate={cursorVariant}
      style={{
        left: cursorXSpring,
        top: cursorYSpring,
      }}
    >
      {/* Detalhe para o I-beam (traços horizontais no topo e base) */}
      {cursorVariant === "text" && (
        <>
          <div className="absolute top-0 left-1/2 -translate-x-1/2 w-3 h-[2px] bg-primary" />
          <div className="absolute bottom-0 left-1/2 -translate-x-1/2 w-3 h-[2px] bg-primary" />
        </>
      )}
    </motion.div>
  );
};

export default CustomCursor;
