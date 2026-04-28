import { AnimatePresence, motion } from "framer-motion";
import {
  Environment,
  GizmoHelper,
  GizmoViewport,
  Grid,
  OrbitControls,
  OrthographicCamera,
  PerspectiveCamera,
} from "@react-three/drei";
import { Canvas, useThree } from "@react-three/fiber";
import { MOUSE } from "three";
import React, { useEffect, useRef, useState } from "react";
import { ICON_STROKE, VIEW_ORIENTATIONS } from "../constants";
import type {
  RenderMode,
  SceneComponent,
  SceneDocument,
  SceneEntity,
  SnapSettings,
  ViewOrientation,
  ViewProjection,
  ViewportRendererProps,
} from "../types";
import { componentValue } from "../borealisCatalog";
import { cameraPositionForView, numpadOrientation, oppositeOrientation } from "../utils/camera";
import { sceneBooleanValue, sceneColorValue, sceneNumberValue } from "../utils/scene";
import { clamp } from "../utils/viewport";
import { StatusPill } from "./shared";
import { Viewport3DEntity } from "./Viewport3DEntity";

export function Viewport3D({
  entities,
  mode,
  preferences,
  scene,
  selectedEntityId,
  onClearSelection,
  onResetCamera,
  onSelectEntity,
  onUpdateTransform,
}: ViewportRendererProps & { scene: SceneDocument }) {
  const orbitRef = useRef<any>(null);
  const [projection, setProjection] = useState<ViewProjection>("perspective");
  const [orientation, setOrientation] = useState<ViewOrientation>("free");
  const [renderMode, setRenderMode] = useState<RenderMode>("light");
  const [radialOpen, setRadialOpen] = useState(false);
  const [gridEditing, setGridEditing] = useState(false);
  const [gridDraft, setGridDraft] = useState("25");
  const [transforming, setTransforming] = useState(false);
  const [snapSettings, setSnapSettings] = useState<SnapSettings>({
    gridSize: 1,
    snapToGrid: preferences.snapMode === "grid" || preferences.snapMode === "grid-object",
    snapToObject: preferences.snapMode === "object" || preferences.snapMode === "grid-object",
  });
  const sceneBackground = sceneColorValue(scene.environment.skybox, "color", "#1c1f26");
  const ambientColor = sceneColorValue(scene.environment.ambient, "color", "#ffffff");
  const ambientIntensity = sceneNumberValue(scene.environment.ambient, "intensity", 0.4);
  const fogEnabled = sceneBooleanValue(scene.environment.fog, "enabled", false);
  const fogColor = sceneColorValue(scene.environment.fog, "color", "#9ca3af");
  const fogDensity = sceneNumberValue(scene.environment.fog, "density", 0.05);
  const postfx = scene.render.postfx ?? {};
  const postfxEnabled =
    sceneBooleanValue(postfx, "fxaa", false) ||
    sceneNumberValue(postfx, "bloom", 0) > 0 ||
    sceneNumberValue(postfx, "vignette", 0) > 0;
  const transformMode = mode === "rotate" ? "rotate" : mode === "scale" ? "scale" : "translate";
  const cameraMode = projection === "isometric" ? "isometric" : projection;

  useEffect(() => {
    setSnapSettings((current) => ({
      ...current,
      snapToGrid: preferences.snapMode === "grid" || preferences.snapMode === "grid-object",
      snapToObject: preferences.snapMode === "object" || preferences.snapMode === "grid-object",
    }));
  }, [preferences.snapMode]);

  useEffect(() => {
    function handleViewportMenu(event: KeyboardEvent) {
      const target = event.target;
      if (
        target instanceof HTMLInputElement ||
        target instanceof HTMLTextAreaElement ||
        target instanceof HTMLSelectElement ||
        (target instanceof HTMLElement && target.isContentEditable)
      ) {
        return;
      }

      if (event.altKey && event.key.toLowerCase() === "z") {
        event.preventDefault();
        setRadialOpen((current) => !current);
      }

      if (radialOpen) {
        const selectedView = VIEW_ORIENTATIONS.find((item) => item.shortcut.toLowerCase() === event.key.toLowerCase());
        if (selectedView) {
          event.preventDefault();
          selectOrientation(selectedView.id);
        }
      }

      if (event.code.startsWith("Numpad")) {
        const nextOrientation = numpadOrientation(event.code, event.ctrlKey);
        if (nextOrientation) {
          event.preventDefault();
          selectOrientation(nextOrientation);
        }

        if (event.code === "Numpad5") {
          event.preventDefault();
          setProjection((current) => (current === "orthographic" ? "perspective" : "orthographic"));
        }

        if (event.code === "Numpad9") {
          event.preventDefault();
          setOrientation((current) => oppositeOrientation(current));
        }
      }

      if (event.key === "Escape") {
        setRadialOpen(false);
      }
    }

    window.addEventListener("keydown", handleViewportMenu);
    return () => window.removeEventListener("keydown", handleViewportMenu);
  }, [radialOpen]);

  function selectProjection(nextProjection: ViewProjection) {
    setProjection(nextProjection);
    if (nextProjection === "isometric") {
      setOrientation("free");
    }
  }

  function selectOrientation(nextOrientation: ViewOrientation) {
    setOrientation(nextOrientation);
    setRadialOpen(false);
  }

  function updateSnapSettings(patch: Partial<SnapSettings>) {
    setSnapSettings((current) => ({ ...current, ...patch }));
  }

  function commitGridSize() {
    const nextSize = clamp(Number(gridDraft) || snapSettings.gridSize, 0.25, 10);
    updateSnapSettings({ gridSize: nextSize });
    setGridDraft(String(nextSize));
    setGridEditing(false);
  }

  return (
    <>
      <Canvas className="viewport-canvas" onPointerMissed={onClearSelection} shadows>
        {projection === "perspective" ? (
          <PerspectiveCamera makeDefault position={[10, 8, 10]} fov={38} />
        ) : (
          <OrthographicCamera makeDefault position={[10, 8, 10]} zoom={58} near={0.1} far={1000} />
        )}
        <ViewportCameraController orientation={orientation} projection={projection} controlsRef={orbitRef} />
        <color attach="background" args={[sceneBackground]} />
        {fogEnabled ? <fog attach="fog" args={[fogColor, 4, Math.max(6, 38 - fogDensity * 24)]} /> : null}
        <ambientLight color={ambientColor} intensity={ambientIntensity} />
        <directionalLight castShadow intensity={0.8} position={[10, 12, 10]} />
        <Environment preset="city" />
        <ViewportSceneLights entities={entities} />
        <Grid
          args={[30, 30]}
          cellColor="#2c2c2c"
          cellSize={snapSettings.gridSize}
          fadeDistance={30}
          fadeStrength={1}
          sectionColor="#353535"
          sectionSize={snapSettings.gridSize * 5}
          infiniteGrid
        />
        <mesh rotation={[Math.PI / 2, 0, 0]} position={[0, -0.02, 0]}>
          <planeGeometry args={[60, 0.015]} />
          <meshBasicMaterial color="#df746c" transparent opacity={0.6} />
        </mesh>
        <mesh rotation={[Math.PI / 2, 0, Math.PI / 2]} position={[0, -0.02, 0]}>
          <planeGeometry args={[60, 0.015]} />
          <meshBasicMaterial color="#72c08b" transparent opacity={0.6} />
        </mesh>
        <group>
          {entities.map((entity) => {
            const selected = entity.id === selectedEntityId;
            return (
              <Viewport3DEntity
                entity={entity}
                key={entity.id}
                mode={mode}
                renderMode={renderMode}
                selected={selected}
                snapSettings={snapSettings}
                transformMode={transformMode}
                entities={entities}
                gizmoSize={preferences.gizmoSize}
                onSelect={onSelectEntity}
                onTransformingChange={setTransforming}
                onUpdateTransform={onUpdateTransform}
              />
            );
          })}
        </group>
        <OrbitControls
          ref={orbitRef}
          enabled={!transforming}
          enableDamping
          makeDefault
          maxDistance={40}
          minDistance={2}
          enablePan
          enableRotate
          enableZoom
          panSpeed={preferences.ptzSpeed}
          rotateSpeed={preferences.ptzSpeed}
          zoomSpeed={preferences.ptzSpeed}
          mouseButtons={{
            MIDDLE: MOUSE.ROTATE,
            RIGHT: MOUSE.PAN,
          }}
          screenSpacePanning
        />

        <GizmoHelper alignment="bottom-right" margin={[40, 40]}>
          <GizmoViewport axisColors={["#df746c", "#72c08b", "#68a4ff"]} labelColor="#efefef" />
        </GizmoHelper>
      </Canvas>
      <div className="viewport-overlay-top">
        <StatusPill>Scene 3D</StatusPill>
        <StatusPill>MMB orbit</StatusPill>
        <StatusPill>Shift+MMB pan</StatusPill>
        <StatusPill>{mode === "select" ? "Select" : transformMode}</StatusPill>
        <StatusPill>{cameraMode}</StatusPill>
        <StatusPill>{orientation === "free" ? "Free" : orientation}</StatusPill>
        {fogEnabled ? <StatusPill>Fog</StatusPill> : null}
        {postfxEnabled ? <StatusPill>PostFX</StatusPill> : null}
        <button className="viewport-reset" onClick={onResetCamera}>Reset</button>
      </div>
      <div className="viewport-view-controls" aria-label="3D viewport view controls">
        <label>
          <span>View</span>
          <select value={orientation} onChange={(event) => selectOrientation(event.target.value as ViewOrientation)}>
            <option value="free">Free</option>
            {VIEW_ORIENTATIONS.map((item) => (
              <option key={item.id} value={item.id}>
                {item.label}
              </option>
            ))}
          </select>
        </label>
        <div className="viewport-viewbar">
          {(["perspective", "orthographic", "isometric"] as ViewProjection[]).map((item) => (
            <button
              className={projection === item ? "active" : ""}
              key={item}
              onClick={() => selectProjection(item)}
              type="button"
            >
              {item === "orthographic" ? "Ortho" : item}
            </button>
          ))}
        </div>
        <label>
          <span>Render</span>
          <select value={renderMode} onChange={(event) => setRenderMode(event.target.value as RenderMode)}>
            <option value="wireframe">Wire</option>
            <option value="color">Color</option>
            <option value="texture">Texture</option>
            <option value="light">Light</option>
          </select>
        </label>
      </div>
      <div className="viewport-snap-panel">
        <label>
          <span>Grid m</span>
          <input
            max={10}
            min={0.25}
            onChange={(event) => {
              updateSnapSettings({ gridSize: Number(event.target.value) });
              setGridDraft(event.target.value);
            }}
            step={0.25}
            type="range"
            value={snapSettings.gridSize}
          />
          {gridEditing ? (
            <input
              className="grid-size-input"
              autoFocus
              onBlur={commitGridSize}
              onChange={(event) => setGridDraft(event.target.value)}
              onKeyDown={(event) => {
                if (event.key === "Enter") commitGridSize();
                if (event.key === "Escape") {
                  setGridDraft(String(snapSettings.gridSize));
                  setGridEditing(false);
                }
              }}
              type="number"
              value={gridDraft}
            />
          ) : (
            <strong onDoubleClick={() => setGridEditing(true)} title="Double-click to type grid size">
              {snapSettings.gridSize}
            </strong>
          )}
        </label>
        <button
          className={snapSettings.snapToGrid ? "active" : ""}
          onClick={() => updateSnapSettings({ snapToGrid: !snapSettings.snapToGrid })}
          type="button"
        >
          Snap grid
        </button>
        <button
          className={snapSettings.snapToObject ? "active" : ""}
          onClick={() => updateSnapSettings({ snapToObject: !snapSettings.snapToObject })}
          type="button"
        >
          Snap object
        </button>
      </div>
      <AnimatePresence>
        {radialOpen ? (
          <motion.div
            animate={{ opacity: 1, scale: 1, x: "-50%", y: "-50%" }}
            className="viewport-radial-menu"
            exit={{ opacity: 0, scale: 0.92, x: "-50%", y: "-50%" }}
            initial={{ opacity: 0, scale: 0.92, x: "-50%", y: "-50%" }}
            transition={{ duration: 0.14, ease: [0.16, 1, 0.3, 1] }}
          >
            <button className="radial-center" onClick={() => setRadialOpen(false)} type="button">
              Alt+Z
            </button>
            {VIEW_ORIENTATIONS.map((item, index) => {
              const angle = -90 + index * 60;
              return (
                <button
                  className={`radial-item ${orientation === item.id ? "active" : ""}`}
                  key={item.id}
                  onClick={() => selectOrientation(item.id)}
                  style={{ "--radial-angle": `${angle}deg` } as React.CSSProperties}
                  type="button"
                >
                  <span>{item.label}</span>
                  <small>{item.shortcut}</small>
                </button>
              );
            })}
          </motion.div>
        ) : null}
      </AnimatePresence>
      <div className="viewport-axis axis-x">X</div>
      <div className="viewport-axis axis-y">Y</div>
      <div className="viewport-axis axis-z">Z</div>
      <div className="viewport-footer">
        <span>Viewport 3D</span>
        <span>{entities.length} objects | Scene Collection</span>
      </div>
    </>
  );
}

function ViewportCameraController({
  controlsRef,
  orientation,
  projection,
}: {
  controlsRef: React.MutableRefObject<any>;
  orientation: ViewOrientation;
  projection: ViewProjection;
}) {
  const { camera } = useThree();

  useEffect(() => {
    const position = cameraPositionForView(projection, orientation);
    camera.position.copy(position);
    camera.up.set(0, 1, 0);
    if (orientation === "top") camera.up.set(0, 0, -1);
    if (orientation === "bottom") camera.up.set(0, 0, 1);
    camera.lookAt(0, 0, 0);
    camera.updateProjectionMatrix();

    const controls = controlsRef.current;
    if (controls) {
      controls.target.set(0, 0, 0);
      controls.update();
    }
  }, [camera, controlsRef, orientation, projection]);

  return null;
}

function ViewportSceneLights({ entities }: { entities: SceneEntity[] }) {
  return (
    <>
      {entities.flatMap((entity) =>
        entity.components
          .filter((component) => component.kind === "directional_light" || component.kind === "point_light" || component.kind === "spot_light")
          .map((component, index) => {
            const position: [number, number, number] = [
              entity.transform.x * 0.04,
              entity.transform.z * 0.04 + 0.45,
              entity.transform.y * 0.04,
            ];
            const color = String(componentValue(component, "color") ?? "#ffffff");
            const intensity = Number(componentValue(component, "intensity") ?? 1);

            if (component.kind === "directional_light") {
              return (
                <directionalLight
                  castShadow={componentValue(component, "shadow") === true}
                  color={color}
                  intensity={intensity}
                  key={`${entity.id}-${component.kind}-${index}`}
                  position={position}
                />
              );
            }

            if (component.kind === "spot_light") {
              return (
                <spotLight
                  angle={(Number(componentValue(component, "angle") ?? 45) * Math.PI) / 180}
                  color={color}
                  distance={Number(componentValue(component, "range") ?? 160) * 0.04}
                  intensity={intensity}
                  key={`${entity.id}-${component.kind}-${index}`}
                  penumbra={0.35}
                  position={position}
                />
              );
            }

            return (
              <pointLight
                color={color}
                distance={Number(componentValue(component, "range") ?? 120) * 0.04}
                intensity={intensity}
                key={`${entity.id}-${component.kind}-${index}`}
                position={position}
              />
            );
          }),
      )}
    </>
  );
}
