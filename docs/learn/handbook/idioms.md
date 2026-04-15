# Idiomas da Trilha Ativa

Este guia resume como escrever codigo Zenith alinhado com a implementacao atual.

## 1. Use a estrutura certa para cada acesso

- use obj.campo para struct
- use map["chave"] para map
- use lista[1] para sequencias

Nao misture semanticas:

- config.status sugere struct, nao mapa
- config["status"] deixa claro que a chave e dinamica

## 2. Prefira Optional e Outcome a null

Politica atual:

- T? = null ainda compila com warning ZT-W001
- T = null gera ZT-S100

Padrao recomendado:

~~~zt
match fs.read_text_file("app.json")
    case Success(txt) => print(txt)
    case Failure(err) => print("falha: " + err)
end
~~~

Para defaults simples:

~~~zt
var nome = nome_opt.unwrap_or("guest")
~~~

## 3. Use validate e where com papeis diferentes

Prefira:

- validate para validadores reaproveitaveis de std.validation
- where para regra booleana livre

~~~zt
struct User
    age: int validate validation.min_value(18), validation.max_value(120)
    name: text validate validation.non_empty where not text.contains(it, "admin")
end
~~~

Evite colocar expressoes arbitrarias dentro de validate.

## 4. @ e para self, nao para metadado

Na trilha ativa:

- @campo e sugar de self.campo
- #[...] e a sintaxe normativa de atributo de declaracao
- @atributo e apenas compatibilidade temporaria com warning ZT-W003

## 5. Escreva metodos e traits com a sintaxe atual

Use:

~~~zt
apply Heal to Player
    func heal(self, amount: int)
        @hp = @hp + amount
    end
end
~~~

Nao use exemplos antigos com apply Trait for Struct.

## 6. UFCS real da trilha ativa

Os helpers abaixo fazem parte do uso diario, mesmo quando a declaracao formal esta no runtime ou no binder:

- len()
- split()
- push(value)
- pop()
- keys()
- is_present()
- is_empty()
- unwrap()
- unwrap_or(default)
- to_text()

~~~zt
var partes = "a,b,c".split()
var total = partes.len()
~~~

## 7. Indexacao: pense em 1 para sequencias

Regra operacional:

- listas e texto sao 1-based
- mapas usam a chave declarada
- 0 em lista ou texto gera warning ZT-W002
- acesso fora dos limites em runtime gera ZT-R011

## 8. Fallback e fluxo

Na trilha ativa, o operador or funciona como fallback ou default no codegen, usando a mesma base de unwrap_or.

Use quando quiser um valor substituto:

~~~zt
var user = os.get_env("USER") or "guest"
~~~

Para logica mais rica, prefira if ou match.
