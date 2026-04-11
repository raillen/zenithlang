import React, { useState, useEffect } from 'react';
import { motion } from 'framer-motion';

const Health = () => {
  const [report, setReport] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetch(reportUrl)
      .then(res => res.json())
      .then(data => {
        setReport(data);
        setLoading(false);
      })
      .catch(err => {
        console.error("Failed to load health report", err);
        setLoading(false);
      });
  }, [reportUrl]);

  if (loading) return (
    <div className="flex items-center justify-center min-h-[60vh]">
      <div className="loading loading-spinner loading-lg text-primary"></div>
    </div>
  );

  if (!report) return (
    <div className="flex flex-col items-center justify-center min-h-[60vh] text-center p-4">
      <h2 className="text-2xl font-bold text-gray-600 mb-2">Relatório de Saúde não encontrado</h2>
      <p className="text-gray-500">Rode 'zt test --report' no CLI para gerar os dados.</p>
    </div>
  );

  const percent = Math.round((report.passed / report.total) * 100);
  const statusColor = report.failed === 0 ? 'text-success' : 'text-error';
  const statusBg = report.failed === 0 ? 'bg-success/10' : 'bg-error/10';

  return (
    <div className="max-w-5xl mx-auto p-8 pt-24">
      <motion.div 
        initial={{ opacity: 0, y: 20 }}
        animate={{ opacity: 1, y: 0 }}
        className="grid grid-cols-1 md:grid-cols-3 gap-6"
      >
        {/* Header Stat */}
        <div className={`md:col-span-3 p-8 rounded-3xl ${statusBg} border border-white/20 shadow-xl backdrop-blur-md`}>
          <div className="flex flex-col md:flex-row justify-between items-center gap-6">
            <div>
              <h1 className="text-4xl font-black text-gray-800 mb-2">Estado da Órbita</h1>
              <p className="text-gray-600 flex items-center gap-2">
                <span className={`w-3 h-3 rounded-full animate-pulse ${report.failed === 0 ? 'bg-success' : 'bg-error'}`}></span>
                Sincronização em tempo real com o Core Zenith
              </p>
            </div>
            <div className="text-center">
              <div className={`text-6xl font-black ${statusColor}`}>{percent}%</div>
              <p className="text-xs font-bold uppercase tracking-widest text-gray-500">Estabilidade Global</p>
            </div>
          </div>
        </div>

        {/* Metrics */}
        <div className="p-6 rounded-3xl bg-white shadow-lg border border-gray-100">
          <div className="text-sm font-bold text-gray-400 uppercase mb-1">Total de Suites</div>
          <div className="text-4xl font-black text-gray-800">{report.total}</div>
          <div className="mt-4 h-2 w-full bg-gray-100 rounded-full overflow-hidden">
            <div className="h-full bg-primary" style={{ width: '100%' }}></div>
          </div>
        </div>

        <div className="p-6 rounded-3xl bg-white shadow-lg border border-gray-100">
          <div className="text-sm font-bold text-gray-400 uppercase mb-1">Sucesso</div>
          <div className="text-4xl font-black text-success">{report.passed}</div>
          <div className="mt-4 h-2 w-full bg-gray-100 rounded-full overflow-hidden">
            <div className="h-full bg-success" style={{ width: `${percent}%` }}></div>
          </div>
        </div>

        <div className="p-6 rounded-3xl bg-white shadow-lg border border-gray-100">
          <div className="text-sm font-bold text-gray-400 uppercase mb-1">Falhas</div>
          <div className="text-4xl font-black text-error">{report.failed}</div>
          <div className="mt-4 h-2 w-full bg-gray-100 rounded-full overflow-hidden">
            <div className="h-full bg-error" style={{ width: `${100 - percent}%` }}></div>
          </div>
        </div>

        {/* Extra Info */}
        <div className="md:col-span-3 bg-gray-800 text-white p-6 rounded-3xl shadow-2xl overflow-hidden relative">
          <div className="relative z-10">
            <h3 className="text-xl font-bold mb-4">Metadados de Auditoria</h3>
            <div className="flex flex-wrap gap-8 text-sm opacity-80">
              <div>
                <span className="block font-bold text-primary">Última Varredura</span>
                {new Date(report.timestamp * 1000).toLocaleString()}
              </div>
              <div>
                <span className="block font-bold text-primary">Duração da Suite</span>
                {report.duration.toFixed(2)}s
              </div>
              <div>
                <span className="block font-bold text-primary">Versão Alvo</span>
                v0.2.8 (Consolidação)
              </div>
            </div>
          </div>
          <div className="absolute top-0 right-0 p-8 opacity-10">
             <svg width="120" height="120" viewBox="0 0 24 24" fill="currentColor"><path d="M12 1L3 5v6c0 5.55 3.84 10.74 9 12 5.16-1.26 9-6.45 9-12V5l-9-4zm0 10.99h7c-.53 4.12-3.28 7.79-7 8.94V12H5V6.3l7-3.11v8.8z"/></svg>
          </div>
        </div>
      </motion.div>
    </div>
  );
};

export default Health;
