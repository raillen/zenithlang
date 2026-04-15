// Simple Service Worker for PWA compliance
const CACHE_NAME = 'zenith-v1';

self.addEventListener('install', (event) => {
  self.skipWaiting();
});

self.addEventListener('activate', (event) => {
  event.waitUntil(clients.claim());
});

self.addEventListener('fetch', (event) => {
  // Pass-through fetch (required for PWA installability)
  event.respondWith(fetch(event.request));
});
