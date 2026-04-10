import { useState, useRef, useEffect } from 'react';
import { ChevronDown, Search, Check } from 'lucide-react';

export interface DropdownOption {
  value: string;
  label: string;
}

interface SearchableDropdownProps {
  options: DropdownOption[];
  value: string;
  onChange: (val: string) => void;
  placeholder?: string;
}

export function SearchableDropdown({ options, value, onChange, placeholder = "Select..." }: SearchableDropdownProps) {
  const [isOpen, setIsOpen] = useState(false);
  const [query, setQuery] = useState('');
  const containerRef = useRef<HTMLDivElement>(null);

  const selectedOption = options.find(o => o.value === value);
  const filteredOptions = options.filter(o => o.label.toLowerCase().includes(query.toLowerCase()));

  useEffect(() => {
    function handleClickOutside(e: MouseEvent) {
      if (containerRef.current && !containerRef.current.contains(e.target as Node)) {
        setIsOpen(false);
      }
    }
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  return (
    <div className="relative w-full" ref={containerRef}>
      <button
        onClick={() => { setIsOpen(!isOpen); setQuery(''); }}
        className="w-full h-9 bg-ide-panel border border-ide-border rounded-lg px-3 flex items-center justify-between text-[12px] text-ide-text hover:border-zinc-400 focus:outline-none transition-colors"
      >
        <span className="truncate">{selectedOption ? selectedOption.label : placeholder}</span>
        <ChevronDown size={14} className="text-ide-text-dim opacity-70" />
      </button>

      {isOpen && (
        <div className="absolute top-10 left-0 w-full bg-ide-bg border border-ide-border rounded-xl shadow-xl z-50 overflow-hidden animate-in fade-in zoom-in-95 duration-100 origin-top">
          <div className="p-2 border-b border-ide-border relative text-ide-text">
             <Search size={12} className="absolute left-4 top-1/2 -translate-y-1/2 text-ide-text-dim" />
             <input 
               type="text" 
               className="w-full bg-ide-panel rounded-md pl-7 pr-3 py-1.5 text-[11px] focus:outline-none placeholder:text-ide-text-dim"
               placeholder="Search..."
               value={query}
               onChange={(e) => setQuery(e.target.value)}
               autoFocus
             />
          </div>
          <div className="max-h-48 overflow-y-auto p-1 no-scrollbar">
            {filteredOptions.length === 0 ? (
              <div className="p-3 text-center text-[10px] text-ide-text-dim">No results found</div>
            ) : (
              filteredOptions.map(option => (
                <button
                  key={option.value}
                  onClick={() => { onChange(option.value); setIsOpen(false); }}
                  className={`w-full text-left px-3 py-2 rounded-lg text-[11px] flex items-center justify-between transition-colors ${value === option.value ? 'bg-primary/10 text-primary font-medium' : 'text-ide-text hover:bg-ide-panel'}`}
                >
                  <span className="truncate">{option.label}</span>
                  {value === option.value && <Check size={12} />}
                </button>
              ))
            )}
          </div>
        </div>
      )}
    </div>
  );
}
