Formato:

* `nome(parâmetros) -> retorno`
* descrição curta

---

# 🧱 Colisão

## Base

* `intersects(a, b) -> bool`
  Testa colisão entre dois shapes.
* `contains(shape, point) -> bool`
  Verifica se um ponto está dentro de um shape.
* `overlap(area) -> Entity[]`
  Retorna entidades dentro de uma área.
* `raycast(origin, direction, maxDistance) -> Hit`
  Retorna o primeiro impacto ao longo de um raio.
* `linecast(start, end) -> Hit`
  Versão simplificada do raycast com dois pontos.
* `sweep(shape, motion) -> Hit`
  Detecta colisão ao mover um shape.

## Intermediário

* `nearest(shape, radius) -> Entity`
  Retorna a entidade mais próxima.
* `query(shape, filter) -> Entity[]`
  Busca entidades com filtro customizado.
* `collisionNormal(hit) -> Vector2`
  Retorna a normal da colisão.
* `penetrationVector(a, b) -> Vector2`
  Vetor mínimo para separar dois corpos.

## Game changer

* `moveAndCollide(entity, motion) -> CollisionInfo`
  Move e retorna colisão com dados completos.
* `moveAndSlide(entity, velocity, up) -> MotionResult`
  Move respeitando superfícies e inclinações.
* `shapeCast(shape, direction, distance) -> Hit`
  Raycast usando o volume do objeto.
* `predictCollision(entity, motion) -> Hit`
  Prevê colisão antes de mover.
* `getContacts(entity) -> Contact[]`
  Lista pontos de contato atuais.

---

# ⚙️ Física

## Base

* `createBody(type) -> Body`
  Cria corpo físico.
* `setVelocity(body, v)`
  Define velocidade.
* `addForce(body, f)`
  Aplica força contínua.
* `addImpulse(body, impulse)`
  Aplica impulso instantâneo.
* `setGravity(body, scale)`
  Ajusta gravidade.
* `setMass(body, mass)`
  Define massa.
* `stepPhysics(dt)`
  Atualiza simulação.

## Intermediário

* `setDrag(body, drag)`
  Define resistência.
* `setFriction(body, friction)`
  Define atrito.
* `setBounciness(body, value)`
  Define restituição.
* `lockRotation(body, bool)`
  Trava rotação.

## Game changer

* `characterBody(config) -> Character`
  Corpo otimizado para personagem.
* `moveCharacter(character, input, dt) -> MotionResult`
  Movimento completo com colisão e física.
* `isOnFloor(character) -> bool`
  Verifica se está no chão.
* `isOnWall(character) -> bool`
  Verifica contato lateral.
* `applyKnockback(body, direction, force)`
  Aplica recuo de gameplay.
* `rewindPhysics(state)`
  Volta estado da simulação.

---

# 🎨 Sprites

## Base

* `draw(texture, position)`
  Desenha textura.
* `drawEx(texture, transform)`
  Desenha com rotação/escala.
* `drawRegion(texture, source, dest)`
  Desenha parte da textura.
* `setColor(sprite, color)`
  Define cor.
* `setFlip(sprite, x, y)`
  Espelha sprite.

## Intermediário

* `createAnimation(frames, fps, loop) -> Animation`
  Cria animação.
* `playAnimation(sprite, anim)`
  Inicia animação.
* `updateAnimation(sprite, dt)`
  Atualiza frame.
* `setAnimationSpeed(sprite, speed)`
  Ajusta velocidade.

## Game changer

* `animationStateMachine() -> Animator`
  Controla estados de animação.
* `setState(animator, state)`
  Troca estado automaticamente.
* `syncAnimationWithVelocity(sprite, velocity)`
  Ajusta animação baseado no movimento.
* `directionalAnimation(sprite, direction)`
  Escolhe animação baseada na direção.
* `spriteBatch() -> Batch`
  Sistema de desenho em lote.

---

# 🧪 Shaders / FX

## Base

* `createShader(vs, fs) -> Shader`
  Cria shader.
* `setUniform(shader, name, value)`
  Define uniform.
* `applyShader(target, shader)`
  Aplica shader.

## Intermediário

* `createMaterial(shader) -> Material`
  Encapsula shader + parâmetros.
* `setMaterialParam(material, name, value)`
  Ajusta material.
* `renderToTexture(size, drawFn) -> Texture`
  Renderiza fora da tela.

## Game changer

* `postProcess(texture, shader) -> Texture`
  Aplica efeito fullscreen.
* `chainEffects(texture, shaders[]) -> Texture`
  Aplica múltiplos efeitos.
* `light2D(config) -> Light`
  Cria luz 2D.
* `applyLighting(scene, lights[])`
  Renderiza iluminação.
* `paletteSwap(texture, palette)`
  Troca cores dinamicamente.

---

# 🕹️ Movimentação

## Base

* `move(entity, delta)`
  Move entidade.
* `setPosition(entity, pos)`
  Define posição.
* `setRotation(entity, angle)`
  Define rotação.
* `moveTowards(current, target, speed, dt)`
  Move gradualmente.

## Intermediário

* `dash(entity, direction, speed, duration)`
  Executa dash.
* `jump(entity, force)`
  Aplica salto.
* `follow(entity, target, speed)`
  Segue alvo.
* `arrive(entity, target, slowRadius)`
  Aproxima com desaceleração.

## Game changer

* `topDownController(config) -> Controller`
  Controlador top-down completo.
* `platformerController(config) -> Controller`
  Controlador de plataforma.
* `updateController(controller, input, dt)`
  Atualiza movimento com feel refinado.
* `applyCoyoteTime(controller, time)`
  Permite pulo após cair.
* `applyJumpBuffer(controller, time)`
  Buffer de input de pulo.
* `avoidObstacles(entity, steering)`
  Desvia automaticamente.

---

# 🤖 IA / Comportamento

## Base

* `lookAt(entity, target)`
  Faz entidade olhar para o alvo.
* `distance(a, b) -> float`
  Calcula distância.

## Intermediário

* `followPath(entity, path, speed)`
  Segue caminho.
* `wander(entity, radius)`
  Movimento aleatório controlado.
* `seek(entity, target)`
  Move em direção ao alvo.
* `flee(entity, target)`
  Afasta do alvo.

## Game changer

* `stateMachine() -> FSM`
  Máquina de estados.
* `setState(fsm, state)`
  Troca estado.
* `behaviorTree() -> BT`
  Cria árvore de comportamento.
* `updateAI(ai, dt)`
  Atualiza lógica.
* `visionCone(entity, angle, distance) -> Entity[]`
  Detecta entidades no campo de visão.
* `hearingSensor(entity, radius) -> Event[]`
  Detecta eventos próximos.

---

# 🎥 Câmera

## Base

* `setCameraPosition(pos)`
  Define posição.
* `setZoom(value)`
  Define zoom.
* `worldToScreen(pos) -> Vector2`
  Converte coordenadas.

## Intermediário

* `follow(target, smooth)`
  Segue alvo suavemente.
* `setBounds(rect)`
  Limita movimento.
* `lerpTo(target, speed)`
  Interpola posição.

## Game changer

* `cameraRig(config) -> CameraRig`
  Sistema completo de câmera.
* `shake(intensity, duration)`
  Aplica tremor.
* `deadzone(rect)`
  Zona morta.
* `lookAhead(offset)`
  Antecipação baseada em movimento.
* `frameTargets(entities[])`
  Enquadra múltiplos objetos.

---

# 🎮 Input

## Base

* `isDown(key) -> bool`
  Tecla pressionada.
* `isPressed(key) -> bool`
  Tecla no frame.
* `getAxis(name) -> float`
  Retorna eixo.

## Intermediário

* `bindAction(name, inputs[])`
  Associa inputs.
* `actionPressed(name) -> bool`
  Verifica ação.

## Game changer

* `inputContext(name)`
  Cria contexto de input.
* `pushContext(name)`
  Ativa contexto.
* `rebind(action, input)`
  Remapeia controle.
* `recordInput() -> Frame`
  Grava input.
* `playInput(frame)`
  Reproduz input.

---

# 🧩 Mundo / Tilemap

## Base

* `loadMap(file) -> Map`
  Carrega mapa.
* `drawMap(map)`
  Desenha mapa.

## Intermediário

* `tileAt(map, x, y)`
  Retorna tile.
* `worldToTile(pos)`
  Converte coordenadas.

## Game changer

* `autoTile(map)`
  Ajusta tiles automaticamente.
* `buildCollision(map)`
  Gera colisão.
* `buildNavMesh(map)`
  Gera navegação.

---

# ✨ Partículas

## Base

* `emit(position)`
  Emite partícula.
* `updateEmitter(dt)`
  Atualiza emissor.

## Game changer

* `attachEmitter(entity)`
  Vincula a entidade.
* `emitOnCollision(info)`
  Emite ao colidir.
* `particlePreset(name)`
  Usa preset pronto.

---

# 🧠 O que realmente faz diferença (resumo)

As funções mais valiosas dessa abordagem:

* `moveAndSlide`
* `characterBody`
* `topDownController / platformerController`
* `animationStateMachine`
* `postProcess + renderToTexture`
* `light2D`
* `cameraRig`
* `visionCone`
* `behaviorTree / FSM`
* `inputContext`

👉 Isso tudo mantém a simplicidade estilo raylib, mas adiciona o que ela não resolve diretamente:

* gameplay feel
* IA prática
* movimentação pronta
* pós-processo fácil
* animação inteligente
