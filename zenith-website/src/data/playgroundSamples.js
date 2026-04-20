const PLAYGROUND_PRESETS = [
  {
    id: 'starter',
    label: 'Starter',
    summary: 'func, var, if, print',
    code: `func main() -> int
    var energia = 96

    if energia > 80
        print("Alta")
    else
        print("Baixa")
    end

    return energia
end`,
  },
  {
    id: 'functions',
    label: 'Functions',
    summary: 'helper + arithmetic',
    code: `func square(value) -> int
    return value * value
end

func main() -> int
    print(square(8))
    return 0
end`,
  },
  {
    id: 'structs',
    label: 'Structs',
    summary: 'struct + constructor',
    code: `struct Pilot
    pub name = "Nadia"
    pub level = 1
end

func main() -> int
    var pilot = Pilot.new({ "name": "Nadia", "level": 4 })
    print(pilot.name)
    print(pilot.level)
    return 0
end`,
  },
];

const PLAYGROUND_INSERTS = [
  {
    id: 'func',
    label: 'func',
    summary: 'Nova função',
    template: `func helper(value) -> int
    return value
end`,
  },
  {
    id: 'if',
    label: 'if',
    summary: 'Condicional',
    template: `if condicao
    print("ok")
else
    print("fallback")
end`,
  },
  {
    id: 'struct',
    label: 'struct',
    summary: 'Estrutura base',
    template: `struct Ship
    pub name = "Orion"
    pub fuel = 100
end`,
  },
  {
    id: 'print',
    label: 'print',
    summary: 'Saída rápida',
    template: `print("debug")`,
  },
];

const presetMap = new Map(PLAYGROUND_PRESETS.map((preset) => [preset.id, preset]));
const insertMap = new Map(PLAYGROUND_INSERTS.map((insert) => [insert.id, insert]));

export const PLAYGROUND_EMBEDS = {
  'zenith-lab': {
    id: 'zenith-lab',
    title: 'Zenith Playground',
    subtitle: 'Edite o código, rode no browser e veja o resultado gerado.',
    hint: 'Dica: o playground executa automaticamente a função main() quando ela existe.',
    shareHint: 'O código fica sincronizado na URL enquanto você edita.',
    presetIds: ['starter', 'functions', 'structs'],
    insertIds: ['func', 'if', 'struct', 'print'],
    defaultPresetId: 'starter',
  },
};

export const getPlaygroundPreset = (presetId) => presetMap.get(presetId) ?? PLAYGROUND_PRESETS[0];
export const getPlaygroundInsert = (insertId) => insertMap.get(insertId) ?? PLAYGROUND_INSERTS[0];

export const getPlaygroundEmbed = (embedId) => {
  const embed = PLAYGROUND_EMBEDS[embedId];

  if (!embed) {
    return null;
  }

  return {
    ...embed,
    inserts: embed.insertIds.map(getPlaygroundInsert),
    presets: embed.presetIds.map(getPlaygroundPreset),
  };
};

export { PLAYGROUND_INSERTS, PLAYGROUND_PRESETS };
