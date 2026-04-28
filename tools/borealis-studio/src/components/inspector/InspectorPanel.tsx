import { Plus, Settings2 } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";
import type {
  ComponentSchema,
  ProjectAsset,
  SceneComponent,
  SceneDocument,
  SceneEntity,
  ScriptDocument,
  Transform3D,
} from "../../types";
import {
  borealisComponents,
  borealisSceneSettings,
  createComponentFromSchema,
  setComponentValue,
} from "../../borealisCatalog";
import { ComponentCard, ComponentFieldControl } from "../ComponentCard";
import { InspectorGroup, PanelHeader, TextField, VectorRow } from "../shared";
import { ObjectHeader } from "./ObjectHeader";
import { TagLayerRow } from "./TagLayerRow";

export function InspectorPanel({
  assets,
  entity,
  scene,
  sceneRootSelected,
  scripts,
  onAttachScript,
  onComponentChange,
  onEntityChange,
  onSceneChange,
  onTransformChange,
}: {
  assets: ProjectAsset[];
  entity: SceneEntity | null;
  scene: SceneDocument;
  sceneRootSelected: boolean;
  scripts: ScriptDocument[];
  onAttachScript: (path: string) => void;
  onComponentChange: (index: number, component: SceneComponent) => void;
  onEntityChange: (patch: Partial<SceneEntity>) => void;
  onSceneChange: (patch: Partial<SceneDocument>) => void;
  onTransformChange: (patch: Partial<Transform3D>) => void;
}) {
  const layers = Array.from(new Set(["default", "world", "world3d", "actors", entity?.layer ?? "default"]).values());
  const addComponentGroups = entity ? componentGroupsForEntity(entity.components) : [];

  return (
    <section className="panel-shell inspector-panel-unity">
      <PanelHeader icon={<Settings2 size={14} strokeWidth={ICON_STROKE} />} meta="Properties" title="Inspector" />
      <div className="inspector-scroll-area">
        {entity ? (
          <>
            <ObjectHeader isSceneRoot={false} layer={entity.layer} name={entity.name} />
            <TagLayerRow
              layer={entity.layer}
              layers={layers}
              onLayerChange={(value) => onEntityChange({ layer: value })}
              onTagsChange={(value) => onEntityChange({ tags: value })}
              tags={entity.tags}
            />
            <InspectorGroup title="Identity">
              <TextField label="Name" onChange={(value) => onEntityChange({ name: value })} value={entity.name} />
            </InspectorGroup>
            <InspectorGroup title="Transform">
              <VectorRow
                label="Position"
                onChange={([x, y, z]) => onTransformChange({ x, y, z })}
                values={[entity.transform.x, entity.transform.y, entity.transform.z]}
              />
              <VectorRow
                label="Rotation"
                onChange={([rotationX, rotationY, rotationZ]) => onTransformChange({ rotationX, rotationY, rotationZ })}
                values={[entity.transform.rotationX, entity.transform.rotationY, entity.transform.rotationZ]}
              />
              <VectorRow
                label="Scale"
                onChange={([scaleX, scaleY, scaleZ]) => onTransformChange({ scaleX, scaleY, scaleZ })}
                step={0.1}
                values={[entity.transform.scaleX, entity.transform.scaleY, entity.transform.scaleZ]}
              />
            </InspectorGroup>
            <InspectorGroup title="Components" noPadding>
              {entity.components.map((component, index) => (
                <ComponentCard
                  assets={assets}
                  component={component}
                  key={`${component.kind}-${component.asset ?? component.script ?? component.profile ?? index}`}
                  scripts={scripts}
                  onChange={(nextComponent) => onComponentChange(index, nextComponent)}
                  onRemove={() => {
                    const next = [...entity.components];
                    next.splice(index, 1);
                    onEntityChange({ components: next });
                  }}
                />
              ))}
            </InspectorGroup>
            <div className="inspector-add-component">
              <Plus size={14} strokeWidth={ICON_STROKE} />
              <select
                aria-label="Add Component"
                value=""
                onChange={(event) => {
                  if (!event.target.value) return;
                  onEntityChange({
                    components: [...entity.components, createComponentFromSchema(event.target.value)],
                  });
                }}
              >
                <option value="">Add Component</option>
                {addComponentGroups.map((group) => (
                  <optgroup key={group.label} label={group.label}>
                    {group.components.map(([kind, schema]) => (
                      <option key={kind} value={kind}>
                        {componentOptionLabel(kind, schema)}
                      </option>
                    ))}
                  </optgroup>
                ))}
              </select>
            </div>
            <InspectorGroup title="Script">
              <label className="text-field">
                <span>Attached script</span>
                <select onChange={(event) => onAttachScript(event.target.value)} value={entity.components.find((component) => component.kind === "script")?.script ?? ""}>
                  <option value="">None</option>
                  {scripts.map((script) => (
                    <option key={script.path} value={script.path}>
                      {script.name}
                    </option>
                  ))}
                </select>
              </label>
            </InspectorGroup>
          </>
        ) : (
          <>
            <ObjectHeader isSceneRoot={sceneRootSelected || !entity} name={scene.name} />
            <SceneSettingsPanel assets={assets} onSceneChange={onSceneChange} scene={scene} scripts={scripts} />
          </>
        )}
      </div>
    </section>
  );
}

function componentGroupsForEntity(existingComponents: SceneComponent[]): Array<{
  label: string;
  components: Array<[string, ComponentSchema]>;
}> {
  const existingKinds = new Set(existingComponents.map((component) => component.kind));
  const groups = new Map<string, Array<[string, ComponentSchema]>>();

  Object.entries(borealisComponents())
    .filter(([kind]) => !existingKinds.has(kind))
    .sort(([leftKind, leftSchema], [rightKind, rightSchema]) =>
      componentOptionLabel(leftKind, leftSchema).localeCompare(componentOptionLabel(rightKind, rightSchema)),
    )
    .forEach(([kind, schema]) => {
      const label = componentCategory(kind, schema);
      const current = groups.get(label) ?? [];
      current.push([kind, schema]);
      groups.set(label, current);
    });

  return Array.from(groups.entries())
    .sort(([left], [right]) => categoryOrder(left) - categoryOrder(right) || left.localeCompare(right))
    .map(([label, components]) => ({ label, components }));
}

function componentOptionLabel(kind: string, schema: ComponentSchema): string {
  return `${schema.label ?? kind} - ${schema.description}`;
}

function componentCategory(kind: string, schema: ComponentSchema): string {
  const raw = `${schema.category ?? ""} ${schema.module} ${kind}`.toLowerCase();
  if (raw.includes("camera")) return "Camera";
  if (raw.includes("audio") || raw.includes("sound")) return "Audio";
  if (raw.includes("render") || raw.includes("model") || raw.includes("sprite") || raw.includes("postfx")) return "Rendering";
  if (raw.includes("physics") || raw.includes("controller") || raw.includes("vehicle") || raw.includes("movement")) return "Gameplay";
  if (raw.includes("ai")) return "AI";
  if (raw.includes("world") || raw.includes("procedural") || raw.includes("weather") || raw.includes("atmosphere")) return "World";
  if (raw.includes("ui") || raw.includes("hud")) return "UI";
  if (raw.includes("debug")) return "Diagnostics";
  if (raw.includes("script") || raw.includes("event")) return "Scripting";
  if (raw.includes("editor")) return "Editor";
  return "Custom";
}

function categoryOrder(label: string): number {
  const order = ["Rendering", "Camera", "Audio", "Gameplay", "AI", "World", "UI", "Scripting", "Diagnostics", "Editor", "Custom"];
  const index = order.indexOf(label);
  return index === -1 ? order.length : index;
}

function SceneSettingsPanel({
  assets,
  scene,
  scripts,
  onSceneChange,
}: {
  assets: ProjectAsset[];
  scene: SceneDocument;
  scripts: ScriptDocument[];
  onSceneChange: (patch: Partial<SceneDocument>) => void;
}) {
  const settings = borealisSceneSettings();
  const entries = Object.entries(settings);
  if (entries.length === 0) return null;
  const environmentValues = scene.environment as unknown as Record<string, Record<string, unknown>>;
  const renderValues = scene.render as unknown as Record<string, Record<string, unknown>>;
  const audioValues = scene.audio as unknown as Record<string, Record<string, unknown>>;

  return (
    <InspectorGroup title="Scene Settings" noPadding>
      {entries.map(([key, schema]) => {
        const section = key.includes("audio") || key === "listener" || key === "mix"
          ? "audio"
          : key.includes("quality") || key.includes("postfx") || key.includes("camera")
            ? "render"
            : "environment";
        const values =
          section === "environment"
            ? environmentValues[key] ?? {}
            : section === "render"
              ? renderValues[key] ?? {}
              : audioValues[key] ?? {};
        const pseudoComponent: SceneComponent = { kind: key, properties: values };

        return (
          <div className="component-card" key={key}>
            <div className="component-card-header">
              <Settings2 size={14} strokeWidth={ICON_STROKE} />
              <strong>{schema.label ?? key}</strong>
            </div>
            <div className="component-card-body">
              <div className="component-fields">
                {schema.fields.map((field) => (
                  <div className="component-field" key={field.key}>
                    <span>{field.label}</span>
                    <ComponentFieldControl
                      assets={assets}
                      component={pseudoComponent}
                      field={field}
                      onChange={(value) => {
                        const nextComponent = setComponentValue(pseudoComponent, field.key, value);
                        const nextValues = nextComponent.properties ?? {};
                        if (section === "environment") {
                          onSceneChange({ environment: { ...scene.environment, [key]: nextValues } });
                        } else if (section === "render") {
                          onSceneChange({ render: { ...scene.render, [key]: nextValues } });
                        } else {
                          onSceneChange({ audio: { ...scene.audio, [key]: nextValues } });
                        }
                      }}
                      scripts={scripts}
                    />
                  </div>
                ))}
              </div>
            </div>
          </div>
        );
      })}
    </InspectorGroup>
  );
}
