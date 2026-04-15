import Prism from 'prismjs';

// Zenith Language Definition for PrismJS
Prism.languages.zenith = {
  'comment': [
    {
      pattern: /(^|[^\\])\/\*[\s\S]*?(?:\*\/|$)/,
      lookbehind: true,
      greedy: true
    },
    {
      pattern: /(^|[^\\:])\/\/.*/,
      lookbehind: true,
      greedy: true
    }
  ],
  'string': {
    pattern: /(["'])(?:\\(?:\r\n|[\s\S])|(?!\1)[^\\\r\n])*\1/,
    greedy: true
  },
  'keyword': /\b(?:def|grid|match|if|else|while|for|in|return|break|continue|import|export|type|interface|outcome|optional|try|panic|test|module)\b/,
  'boolean': /\b(?:true|false|none|null)\b/,
  'function': /\b\w+(?=\s*\()/,
  'number': /-?\b\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b/,
  'operator': /[?]=?|->|=>|==|!=|<=|>=|&&|\|\||[-+*/%!=<>]/,
  'punctuation': /[\[\](){},.:]/
};

Prism.languages.zt = Prism.languages.zenith;

export default Prism.languages.zenith;
