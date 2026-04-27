import os
import glob
import re
import time
from deep_translator import GoogleTranslator

SOURCE_DIR = 'docs/public'
EN_DIR = 'docs/public/en'
ES_DIR = 'docs/public/es'
JP_DIR = 'docs/jp'

def translate_text(text, target_lang):
    translator = GoogleTranslator(source='pt', target=target_lang)
    
    parts = re.split(r'(```.*?```|`[^`]+`)', text, flags=re.DOTALL)
    
    translated_parts = []
    for i, part in enumerate(parts):
        if i % 2 == 1:
            # This is a code block or inline code, keep it unchanged
            translated_parts.append(part)
        else:
            # This is text, translate paragraph by paragraph
            if not part.strip():
                translated_parts.append(part)
                continue
                
            # Split by double newline to preserve formatting
            # Also keep single newlines intact if possible, but splitting by \n\n is safer for APIs
            # We use a custom split that preserves the exact whitespace
            paragraphs = re.split(r'(\n\n+)', part)
            
            translated_paragraphs = []
            for p in paragraphs:
                if not p.strip():
                    translated_paragraphs.append(p)
                    continue
                try:
                    time.sleep(0.1)
                    # Translate in chunks if paragraph is too long, but usually it's fine
                    # GoogleTranslator handles up to 5000 chars
                    translated = translator.translate(p)
                    if translated is None:
                        translated_paragraphs.append(p)
                    else:
                        translated_paragraphs.append(translated)
                except Exception as e:
                    print(f"Translation failed for a chunk: {e}")
                    translated_paragraphs.append(p)
            translated_parts.append(''.join(translated_paragraphs))
            
    return ''.join(translated_parts)

def process_file(file_path):
    rel_path = os.path.relpath(file_path, SOURCE_DIR)
    
    # Check if we should skip
    parts = rel_path.split(os.sep)
    if parts[0] in ['en', 'es']:
        return
        
    en_path = os.path.join(EN_DIR, rel_path)
    es_path = os.path.join(ES_DIR, rel_path)
    ja_path = os.path.join(JP_DIR, rel_path)
    
    if os.path.exists(en_path) and os.path.exists(es_path) and os.path.exists(ja_path):
        print(f"Skipping {file_path} (already translated)")
        return
        
    print(f"Processing {file_path}...")
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    # Translate to English
    if not os.path.exists(en_path):
        print(f"  Translating to EN...")
        en_content = translate_text(content, 'en')
        os.makedirs(os.path.dirname(en_path), exist_ok=True)
        with open(en_path, 'w', encoding='utf-8') as f:
            f.write(en_content)
        
    # Translate to Spanish
    if not os.path.exists(es_path):
        print(f"  Translating to ES...")
        es_content = translate_text(content, 'es')
        os.makedirs(os.path.dirname(es_path), exist_ok=True)
        with open(es_path, 'w', encoding='utf-8') as f:
            f.write(es_content)

    # Translate to Japanese
    if not os.path.exists(ja_path):
        print(f"  Translating to JA...")
        ja_content = translate_text(content, 'ja')
        os.makedirs(os.path.dirname(ja_path), exist_ok=True)
        with open(ja_path, 'w', encoding='utf-8') as f:
            f.write(ja_content)

def main():
    os.makedirs(EN_DIR, exist_ok=True)
    os.makedirs(ES_DIR, exist_ok=True)
    os.makedirs(JP_DIR, exist_ok=True)
    
    files = glob.glob(os.path.join(SOURCE_DIR, '**/*.md'), recursive=True)
    for f in files:
        process_file(f)
        
if __name__ == '__main__':
    main()
