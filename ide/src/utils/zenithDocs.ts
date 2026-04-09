export interface BuiltinDoc {
  description: string;
  signature: string;
  example: string;
}

export const ZENITH_BUILTINS: Record<string, BuiltinDoc> = {
  "io.print": {
    signature: "io.print(value: any): void",
    description: "Imprime um valor no terminal padrão da IDE.",
    example: 'io.print("Olá Zenith!")'
  },
  "math.sin": {
    signature: "math.sin(x: number): number",
    description: "Retorna o seno de um número (em radianos).",
    example: "let s = math.sin(3.14)"
  },
  "outcome.ok": {
    signature: "outcome.ok<T>(value: T): Outcome<T, any>",
    description: "Cria um objeto Outcome representando um sucesso.",
    example: "return outcome.ok(true)"
  },
  "optional.some": {
    signature: "optional.some<T>(value: T): Optional<T>",
    description: "Cria um objeto Optional contendo um valor.",
    example: "let opt = optional.some(42)"
  }
};
