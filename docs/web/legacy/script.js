document.addEventListener('DOMContentLoaded', () => {
    const docContent = document.getElementById('doc-content');
    const sidebarMenu = document.getElementById('sidebar-menu');
    const tocRight = document.getElementById('toc-right');
    const themeToggle = document.getElementById('theme-toggle');
    const htmlDir = document.documentElement;

    // 1. Icon Mapping (Heroicons Solid)
    const iconMap = {
        "0. Guia de Estudo": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path d="M11.7 2.805a.75.75 0 0 1 .6 0l9.435 4.193a.75.75 0 0 1 0 1.372l-9.435 4.193a.75.75 0 0 1-.6 0L2.265 8.37a.75.75 0 0 1 0-1.372l9.435-4.193Z" /><path d="M2.265 11.23a.75.75 0 0 1 .735.032l9 4.193a.75.75 0 0 0 .6 0l9-4.193a.75.75 0 0 1 .735 1.306l-9 4.193a2.25 2.25 0 0 1-1.8 0l-9-4.193a.75.75 0 0 1 .73-.137Z" /><path d="M2.265 14.23a.75.75 0 0 1 .735.032l9 4.193a.75.75 0 0 0 .6 0l9-4.193a.75.75 0 0 1 .735 1.306l-9 4.193a2.25 2.25 0 0 1-1.8 0l-9-4.193a.75.75 0 0 1 .73-.137Z" /></svg>`,
        "1. Fundamentos": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path d="M12.375 3.033a.75.75 0 0 0-.75 0L3.75 7.5v9a.75.75 0 0 0 .375.65l7.5 4.333a.75.75 0 0 0 .75 0l7.5-4.333a.75.75 0 0 0 .375-.65v-9l-7.875-4.467Z" /></svg>`,
        "2. Lógica e Fluxo": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path fill-rule="evenodd" d="M14.615 1.595a.75.75 0 01.359.852L12.982 9.75h7.268a.75.75 0 01.548 1.262l-10.5 11.25a.75.75 0 01-1.272-.71l1.992-7.302H3.75a.75.75 0 01-.548-1.262L13.702 1.777a.75.75 0 01.913-.182z" clip-rule="evenodd" /></svg>`,
        "3. Coleções e Dados": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path d="M21 6.375c0 2.623-4.03 4.75-9 4.75s-9-2.127-9-4.75S7.03 1.625 12 1.625s9 2.127 9 4.75ZM3.75 9.769c.124.085.256.165.396.24 1.893.996 4.654 1.616 7.854 1.616s5.961-.62 7.854-1.616a2.449 2.449 0 0 0 .396-.24V12c0 2.623-4.03 4.75-9 4.75s-9-2.127-9-4.75V9.769Zm0 5.394c.124.085.256.165.396.24 1.893.996 4.654 1.616 7.854 1.616s5.961-.62 7.854-1.616a2.449 2.449 0 0 0 .396-.24V17.375c0 2.623-4.03 4.75-9 4.75s-9-2.127-9-4.75v-2.212Z" /></svg>`,
        "4. Funções e Organização": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path fill-rule="evenodd" d="M3 6a3 3 0 0 1 3-3h12a3 3 0 0 1 3 3v12a3 3 0 0 1-3 3H6a3 3 0 0 1-3-3V6Zm14.25 6a.75.75 0 0 1-.75.75H7.5a.75.75 0 0 1 0-1.5h6a.75.75 0 0 1 .75.75Zm0 3a.75.75 0 0 1-.75.75H7.5a.75.75 0 0 1 0-1.5h6a.75.75 0 0 1 .75.75Zm0-6a.75.75 0 0 1-.75.75H7.5a.75.75 0 0 1 0-1.5h6a.75.75 0 0 1 .75.75Z" clip-rule="evenodd" /></svg>`,
        "5. Composição": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path d="M11 2.25a.75.75 0 0 1 1.5 0V3h.75a3 3 0 0 1 3 3v2.25h.75a0.75.75 0 0 1 0 1.5h-.75V12h.75a.75.75 0 0 1 0 1.5h-.75V15.75a3 3 0 0 1-3 3h-.75v.75a.75.75 0 0 1-1.5 0v-.75H9a3 3 0 0 1-3-3v-2.25H5.25a.75.75 0 0 1 0-1.5H6V9.75H5.25a.75.75 0 0 1 0-1.5H6V6a3 3 0 0 1 3-3h.75v-.75Z" /></svg>`,
        "6. Reatividade e Erros": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path d="M12 4.5a.75.75 0 0 1 .75.75v3.43c2.72.33 4.95 2.45 5.43 5.14a.75.75 0 0 1-1.48.26 4.001 4.001 0 0 0-7.38 0 .75.75 0 0 1-1.48-.26c.48-2.69 2.71-4.81 5.43-5.14V5.25A.75.75 0 0 1 12 4.5Z" /><path d="M2.52 13.92a.75.75 0 0 1 1.06 0 9.001 9.001 0 0 1 12.72 0 .75.75 0 0 1-1.06 1.06 7.5 7.5 0 0 0-10.6 0 .75.75 0 0 1-1.06-1.06Z" /><path d="M5.385 16.785a.75.75 0 0 1 1.06 0 4.5 4.5 0 0 1 6.36 0 .75.75 0 0 1-1.06 1.06 3 3 0 0 0-4.24 0 .75.75 0 0 1-1.06-1.06Z" /><path d="M12 18.75a1.125 1.125 0 1 0 0 2.25 1.125 1.125 0 0 0 0-2.25Z" /></svg>`,
        "7. Contratos e Metadados": `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" class="w-5 h-5"><path fill-rule="evenodd" d="M12.516 2.17a.75.75 0 0 0-1.032 0 11.209 11.209 0 0 1-7.877 3.08.75.75 0 0 0-.722.515A12.74 12.74 0 0 0 2.25 9.75c0 5.942 4.064 10.933 9.563 12.348a.749.749 0 0 0 .374 0c5.499-1.415 9.563-6.406 9.563-12.348 0-1.39-.223-2.73-.635-3.985a.75.75 0 0 0-.722-.516l-.143.001c-2.996 0-5.717-1.17-7.734-3.08ZM15.78 9.22a.75.75 0 0 0-1.06 0l-3.22 3.22-1.47-1.47a.75.75 0 0 0-1.06 1.06l2 2a.75.75 0 0 0 1.06 0l3.75-3.75a.75.75 0 0 0 0-1.06Z" clip-rule="evenodd" /></svg>`
    };

    const getIcon = (title) => {
        for (const [key, svg] of Object.entries(iconMap)) {
            if (title.includes(key)) return svg;
        }
        return ""; // Fallback
    };

    // 1. Theme Toggle Logic
    const toggleTheme = () => {
        const currentTheme = htmlDir.getAttribute('data-theme');
        const newTheme = currentTheme === 'light' ? 'dark' : 'light';
        htmlDir.setAttribute('data-theme', newTheme);
        localStorage.setItem('zenith-theme', newTheme);
        updateThemeIcons(newTheme);
    };

    const updateThemeIcons = (theme) => {
        const sun = document.getElementById('sun-icon');
        const moon = document.getElementById('moon-icon');
        if (theme === 'dark') {
            sun.classList.remove('hidden');
            moon.classList.add('hidden');
        } else {
            sun.classList.add('hidden');
            moon.classList.remove('hidden');
        }
    };

    themeToggle.addEventListener('click', toggleTheme);
    const savedTheme = localStorage.getItem('zenith-theme') || 'light';
    htmlDir.setAttribute('data-theme', savedTheme);
    updateThemeIcons(savedTheme);

    // 2. Load Markdown Content
    async function loadContent() {
        try {
            const response = await fetch('../language-spec/syntax.md');
            if (!response.ok) throw new Error("Fetch failed");
            const text = await response.text();
            renderMarkdown(text);
        } catch (err) {
            console.warn("Fetch falhou. O site deve ser aberto via servidor local.");
            docContent.innerHTML = `
                <div class="alert alert-warning shadow-lg">
                    <svg xmlns="http://www.w3.org/2000/svg" class="stroke-current flex-shrink-0 h-6 w-6" fill="none" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" /></svg>
                    <span><strong>Atenção:</strong> Abra o site via servidor para carregar a documentação.</span>
                </div>
            `;
        }
    }

    function renderMarkdown(md) {
        marked.setOptions({
            highlight: function(code, lang) {
                if (Prism.languages[lang]) return Prism.highlight(code, Prism.languages[lang], lang);
                return code;
            }
        });

        const cleanMd = md.replace(/^---[\s\S]*?---/, '');
        docContent.innerHTML = marked.parse(cleanMd);
        
        // Injetar ícones nos H2 do conteúdo central também
        document.querySelectorAll('#doc-content h2').forEach(h2 => {
            const icon = getIcon(h2.innerText);
            if (icon) {
                h2.innerHTML = `<span class="inline-flex items-center gap-3 text-primary">${icon} ${h2.innerText}</span>`;
            }
        });

        generateSidebar(cleanMd);
        generateTOC();
        if (typeof Prism !== 'undefined') Prism.highlightAll();
    }

    function generateSidebar(md) {
        const lines = md.split('\n');
        const h2s = lines.filter(l => l.startsWith('## ')).map(l => l.replace('## ', '').trim());
        
        sidebarMenu.innerHTML = h2s.map((title, index) => {
            if (title === "Parabéns!") return ""; // Pula a seção final no menu lateral
            const icon = getIcon(title);
            return `
                <li>
                    <details open>
                        <summary class="hover:bg-primary/10 transition-colors py-3 px-4 rounded-lg">
                            <span class="flex items-center gap-3 font-medium text-base-content/80">
                                <span class="text-primary/70">${icon}</span>
                                ${title}
                            </span>
                        </summary>
                        <ul id="submenu-${index}" class="menu-sm mt-1 ml-4 border-l border-base-content/10"></ul>
                    </details>
                </li>
            `;
        }).join('');

        let currentH2 = -1;
        lines.forEach(line => {
            if (line.startsWith('## ')) {
                currentH2++;
            } else if (line.startsWith('### ') && currentH2 >= 0) {
                const title = line.replace('### ', '').trim();
                const id = title.toLowerCase().replace(/[^\w]/g, '-');
                const submenu = document.getElementById(`submenu-${currentH2}`);
                if (submenu) {
                    submenu.innerHTML += `<li><a href="#${id}" class="hover:text-primary transition-colors py-2">${title}</a></li>`;
                }
            }
        });
    }

    function generateTOC() {
        const h3s = document.querySelectorAll('#doc-content h3');
        tocRight.innerHTML = Array.from(h3s).map(h3 => {
            const title = h3.innerText;
            const id = title.toLowerCase().replace(/[^\w]/g, '-');
            h3.id = id;
            return `<li><a href="#${id}" class="py-2 border-l-2 border-transparent hover:border-primary px-4 block transition-all hover:bg-base-content/5">${title}</a></li>`;
        }).join('');
    }

    loadContent();
});
