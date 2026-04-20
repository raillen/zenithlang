import React from 'react';
import { motion } from 'framer-motion';
import { ArrowRight, BookOpen, LinkSimple } from '@phosphor-icons/react';
import CodePlayground from '../components/CodePlayground';

const PlaygroundPage = ({ onOpenDocs }) => {
  return (
    <motion.div
      data-z-id="playground-page"
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      exit={{ opacity: 0 }}
      className="page-playground min-h-[calc(100dvh-140px)] px-4 py-10 sm:px-8 md:px-12 md:py-14"
    >
      <div className="mx-auto flex w-full max-w-[1360px] flex-col gap-10">
        <section className="grid gap-8 border-b border-black/5 pb-8 lg:grid-cols-[minmax(0,1.15fr)_minmax(280px,0.85fr)] lg:items-end">
          <div className="max-w-[70ch]">
            <div className="flex items-center gap-2 text-[11px] font-bold uppercase tracking-[0.18em] text-neutral/45">
              <span className="inline-flex h-2 w-2 rounded-full bg-primary" aria-hidden="true" />
              Dedicated playground
            </div>

            <h1 className="mt-3 text-4xl font-semibold tracking-tight text-[#111111] sm:text-5xl">
              Escreva, rode e compartilhe snippets Zenith sem sair da interface principal.
            </h1>

            <p className="mt-4 max-w-[60ch] text-base leading-8 text-neutral/70">
              Esta tela concentra o editor, a compilacao e a saida de execucao em um fluxo unico.
              Quando precisar de contexto extra, a documentação detalhada continua disponivel logo ao lado.
            </p>
          </div>

          <div className="flex flex-col gap-3 lg:items-end">
            <button
              type="button"
              onClick={onOpenDocs}
              className="inline-flex items-center justify-center gap-2 rounded-md bg-[#111111] px-4 py-3 text-sm font-semibold text-white transition-all hover:-translate-y-[1px]"
            >
              <BookOpen size={16} weight="fill" />
              Abrir guia na docs
              <ArrowRight size={16} weight="bold" />
            </button>

            <p className="flex items-center gap-2 text-sm text-neutral/55">
              <LinkSimple size={15} />
              O botão Link do playground copia a URL com o snippet atual.
            </p>
          </div>
        </section>

        <CodePlayground embedId="zenith-lab" />
      </div>
    </motion.div>
  );
};

export default PlaygroundPage;
