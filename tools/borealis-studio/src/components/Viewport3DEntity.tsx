import { TransformControls } from "@react-three/drei";
import React, { useEffect, useRef, useState } from "react";
import type { Group } from "three";
import type { RenderMode, SceneComponent, SceneEntity, SnapSettings, StudioMode, Transform3D } from "../types";
import { componentValue } from "../borealisCatalog";
import { snapTransform } from "../utils/viewport";
import { clamp } from "../utils/viewport";

export function Viewport3DEntity({
  entities,
  entity,
  gizmoSize,
  mode,
  renderMode,
  selected,
  snapSettings,
  transformMode,
  onSelect,
  onTransformingChange,
  onUpdateTransform,
}: {
  entities: SceneEntity[];
  entity: SceneEntity;
  gizmoSize: number;
  mode: StudioMode;
  renderMode: RenderMode;
  selected: boolean;
  snapSettings: SnapSettings;
  transformMode: "translate" | "rotate" | "scale";
  onSelect: (entity: SceneEntity) => void;
  onTransformingChange: (active: boolean) => void;
  onUpdateTransform: (id: string, transform: Transform3D) => void;
}) {
  const groupRef = useRef<Group | null>(null);
  const [controlTarget, setControlTarget] = useState<Group | null>(null);
  const cameraComponent = entity.components.find((component) => component.kind === "camera3d" || component.kind === "camera2d");
  const lightComponent = entity.components.find((component) =>
    component.kind === "directional_light" || component.kind === "point_light" || component.kind === "spot_light",
  );
  const audioComponent = entity.components.find((component) => component.kind === "audio" || component.kind === "audio3d");
  const isCamera = Boolean(cameraComponent);
  const isLight = Boolean(lightComponent);
  const isAudio = Boolean(audioComponent);
  const isModel = entity.components.some((component) => component.kind === "mesh3d" || component.kind === "model3d");
  const color = selected ? "#68a4ff" : isLight ? "#f5d36b" : isAudio ? "#b48cff" : isCamera ? "#72c08b" : isModel ? "#d9a05c" : "#9aa4b5";
  const position: [number, number, number] = [
    entity.transform.x * 0.04,
    entity.transform.z * 0.04 + 0.45,
    entity.transform.y * 0.04,
  ];
  const scale: [number, number, number] = [
    Math.max(0.25, entity.transform.scaleX),
    Math.max(0.25, entity.transform.scaleZ),
    Math.max(0.25, entity.transform.scaleY),
  ];

  useEffect(() => {
    if (groupRef.current) {
      setControlTarget(groupRef.current);
    }
  }, []);

  useEffect(() => {
    if (!selected || mode === "select") {
      onTransformingChange(false);
    }
  }, [mode, onTransformingChange, selected]);

  useEffect(() => {
    const group = groupRef.current;
    if (!group) return;
    group.position.set(position[0], position[1], position[2]);
    group.rotation.set(
      (entity.transform.rotationX * Math.PI) / 180,
      (entity.transform.rotationY * Math.PI) / 180,
      (entity.transform.rotationZ * Math.PI) / 180,
    );
    group.scale.set(scale[0], scale[1], scale[2]);
  }, [
    entity.transform.rotationX,
    entity.transform.rotationY,
    entity.transform.rotationZ,
    position[0],
    position[1],
    position[2],
    scale[0],
    scale[1],
    scale[2],
  ]);

  function commitTransform() {
    const group = groupRef.current;
    if (!group) return;
    const nextTransform = snapTransform(
      {
        x: Math.round(group.position.x / 0.04),
        y: Math.round(group.position.z / 0.04),
        z: Math.round((group.position.y - 0.45) / 0.04),
        rotationX: Math.round((group.rotation.x * 180) / Math.PI),
        rotationY: Math.round((group.rotation.y * 180) / Math.PI),
        rotationZ: Math.round((group.rotation.z * 180) / Math.PI),
        scaleX: Number(Math.max(0.1, group.scale.x).toFixed(2)),
        scaleY: Number(Math.max(0.1, group.scale.z).toFixed(2)),
        scaleZ: Number(Math.max(0.1, group.scale.y).toFixed(2)),
      },
      entity,
      entities,
      snapSettings,
    );
    onUpdateTransform(entity.id, nextTransform);
  }

  const entityNode = (
    <group ref={groupRef} position={position} rotation={[
      (entity.transform.rotationX * Math.PI) / 180,
      (entity.transform.rotationY * Math.PI) / 180,
      (entity.transform.rotationZ * Math.PI) / 180,
    ]} scale={scale}>
      <ViewportEntityGizmo
        audioComponent={audioComponent}
        cameraComponent={cameraComponent}
        color={color}
        entity={entity}
        lightComponent={lightComponent}
        renderMode={renderMode}
        onSelect={onSelect}
      />
      {selected ? (
        <ViewportSelectionOutline audioComponent={audioComponent} cameraComponent={cameraComponent} lightComponent={lightComponent} />
      ) : null}
    </group>
  );

  if (!selected || mode === "select" || !controlTarget) return entityNode;

  return (
    <>
      {entityNode}
      <TransformControls
        mode={transformMode}
        object={controlTarget}
        onMouseDown={() => onTransformingChange(true)}
        onMouseUp={() => {
          onTransformingChange(false);
          commitTransform();
        }}
        size={gizmoSize}
      />
    </>
  );
}

function ViewportEntityGizmo({
  audioComponent,
  cameraComponent,
  color,
  entity,
  lightComponent,
  renderMode,
  onSelect,
}: {
  audioComponent?: SceneComponent;
  cameraComponent?: SceneComponent;
  color: string;
  entity: SceneEntity;
  lightComponent?: SceneComponent;
  renderMode: RenderMode;
  onSelect: (entity: SceneEntity) => void;
}) {
  const clickProps = {
    onClick: (event: any) => {
      event.stopPropagation();
      onSelect(entity);
    },
  };

  if (cameraComponent) {
    const fov = Number(componentValue(cameraComponent, "fov") ?? 60);
    const length = clamp(fov / 90, 0.45, 1.4);
    return (
      <group>
        <mesh {...clickProps} rotation={[Math.PI / 2, 0, Math.PI / 4]}>
          <coneGeometry args={[0.42, 0.78, 4]} />
          <meshBasicMaterial color={color} wireframe />
        </mesh>
        <mesh {...clickProps} position={[0, 0, -length * 0.56]} rotation={[Math.PI / 2, 0, 0]}>
          <coneGeometry args={[length * 0.52, length, 4, 1, true]} />
          <meshBasicMaterial color={color} transparent opacity={0.18} wireframe />
        </mesh>
      </group>
    );
  }

  if (lightComponent) {
    const lightColor = String(componentValue(lightComponent, "color") ?? color);
    const range = Number(componentValue(lightComponent, "range") ?? 120) * 0.04;
    if (lightComponent.kind === "directional_light") {
      return (
        <group>
          <mesh {...clickProps}>
            <sphereGeometry args={[0.28, 18, 12]} />
            <meshBasicMaterial color={lightColor} />
          </mesh>
          <mesh {...clickProps} position={[0, 0, -0.55]} rotation={[Math.PI / 2, 0, 0]}>
            <coneGeometry args={[0.22, 0.65, 16]} />
            <meshBasicMaterial color={lightColor} wireframe />
          </mesh>
        </group>
      );
    }

    if (lightComponent.kind === "spot_light") {
      const angle = Number(componentValue(lightComponent, "angle") ?? 45);
      const coneRadius = clamp((angle / 90) * range * 0.35, 0.18, 2.4);
      return (
        <group>
          <mesh {...clickProps}>
            <sphereGeometry args={[0.22, 18, 12]} />
            <meshBasicMaterial color={lightColor} />
          </mesh>
          <mesh {...clickProps} position={[0, 0, -range * 0.45]} rotation={[Math.PI / 2, 0, 0]}>
            <coneGeometry args={[coneRadius, Math.max(0.4, range), 24, 1, true]} />
            <meshBasicMaterial color={lightColor} transparent opacity={0.28} wireframe />
          </mesh>
        </group>
      );
    }

    return (
      <group>
        <mesh {...clickProps}>
          <sphereGeometry args={[0.24, 18, 12]} />
          <meshBasicMaterial color={lightColor} />
        </mesh>
        <mesh {...clickProps}>
          <sphereGeometry args={[Math.max(0.35, range), 24, 16]} />
          <meshBasicMaterial color={lightColor} transparent opacity={0.18} wireframe />
        </mesh>
      </group>
    );
  }

  if (audioComponent) {
    const range = Number(componentValue(audioComponent, "range") ?? 80) * 0.04;
    return (
      <group>
        <mesh {...clickProps}>
          <sphereGeometry args={[0.22, 18, 12]} />
          <meshBasicMaterial color={color} />
        </mesh>
        <mesh {...clickProps}>
          <sphereGeometry args={[Math.max(0.35, range), 24, 16]} />
          <meshBasicMaterial color={color} transparent opacity={0.16} wireframe />
        </mesh>
        <mesh {...clickProps} position={[0, 0.38, 0]}>
          <torusGeometry args={[0.24, 0.025, 8, 24]} />
          <meshBasicMaterial color={color} />
        </mesh>
      </group>
    );
  }

  return (
    <mesh
      castShadow
      {...clickProps}
    >
      <boxGeometry args={[0.9, 0.9, 0.9]} />
      <ViewportEntityMaterial color={color} renderMode={renderMode} />
    </mesh>
  );
}

function ViewportEntityMaterial({ color, renderMode }: { color: string; renderMode: RenderMode }) {
  if (renderMode === "wireframe") {
    return <meshBasicMaterial color={color} wireframe />;
  }

  if (renderMode === "color") {
    return <meshBasicMaterial color={color} />;
  }

  if (renderMode === "texture") {
    return <meshStandardMaterial color={color} roughness={0.72} metalness={0.04} />;
  }

  return <meshStandardMaterial color={color} roughness={0.4} metalness={0.2} />;
}

function ViewportSelectionOutline({
  audioComponent,
  cameraComponent,
  lightComponent,
}: {
  audioComponent?: SceneComponent;
  cameraComponent?: SceneComponent;
  lightComponent?: SceneComponent;
}) {
  if (cameraComponent) {
    return (
      <mesh rotation={[Math.PI / 2, 0, Math.PI / 4]} scale={[1.08, 1.08, 1.08]}>
        <coneGeometry args={[0.42, 0.78, 4]} />
        <meshBasicMaterial color="#68a4ff" wireframe />
      </mesh>
    );
  }

  if (lightComponent || audioComponent) {
    return (
      <mesh>
        <sphereGeometry args={[0.38, 18, 12]} />
        <meshBasicMaterial color="#68a4ff" wireframe />
      </mesh>
    );
  }

  return (
    <mesh scale={[1.05, 1.05, 1.05]}>
      <boxGeometry args={[0.9, 0.9, 0.9]} />
      <meshBasicMaterial color="#68a4ff" wireframe />
    </mesh>
  );
}
