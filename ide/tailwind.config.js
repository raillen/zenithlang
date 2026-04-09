/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        xcode: {
          sidebar: 'rgba(38, 38, 38, 0.7)',
          editor: 'rgba(30, 30, 30, 0.5)',
          toolbar: 'rgba(45, 45, 45, 0.8)',
          accent: '#007AFF',
          border: 'rgba(255, 255, 255, 0.1)',
        }
      },
      backgroundImage: {
        'glass-gradient': 'linear-gradient(to bottom, rgba(255, 255, 255, 0.05), rgba(255, 255, 255, 0))',
      }
    },
  },
  plugins: [],
}
