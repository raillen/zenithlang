import { Settings2 } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../constants";
import type { ProjectAsset, SceneComponent, SceneDocument, SceneEntity, ScriptDocument, Transform3D } from "../types";
import {
  borealisComponents,
  borealisSceneSettings,
  componentValue,
  createComponentFromSchema,
  setComponentValue,
} from "../borealisCatalog";
import { entityScript } from "../utils/entity";
import { sceneSettingSection, sceneSettingValues } from "../utils/scene";
import { ComponentCard, ComponentFieldControl } from "./ComponentCard";
import { InspectorGroup, PanelHeader, TextField, VectorRow } from "./shared";

export function InspectorPanel({
  assets,
  entity,
  scene,
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
  scripts: ScriptDocument[];
  onAttachScript: (path: string) => void;
  onComponentChange: (index: number, component: SceneComponent) => void;
  onEntityChange: (patch: Partial<SceneEntity>) => void;
  onSceneChange: (patch: Partial<SceneDocument>) => void;
  onTransformChange: (patch: Partial<Transform3D>) => void;
}) {
  if (!entity) {
    return (
      <aside className="side-panel inspector-panel">
        <PanelHeader icon={<Settings2 size={15} strokeWidth={ICON_STROKE} />} title="Inspector" meta="Properties" />
        <div className="inspector-scroll">
          <div className="empty-state">No object selected.</div>
          <SceneSettingsPanel assets={assets} scene={scene} scripts={scripts} onSceneChange={onSceneChange} />
        </div>
      </aside>
    );
  }

  const attachedScript = entityScript(entity) ?? "";

  return (
    <aside className="side-panel inspector-panel">
      <PanelHeader icon={<Settings2 size={15} strokeWidth={ICON_STROKE} />} title="Inspector" meta={entity.layer} />
      <div className="inspector-scroll">
        <InspectorGroup title="Identity">
          <TextField label="Name" value={entity.name} onChange={(value) => onEntityChange({ name: value })} />
          <TextField label="Layer" value={entity.layer} onChange={(value) => onEntityChange({ layer: value })} />
          <TextField
            label="Tags"
            value={entity.tags.join(", ")}
            onChange={(value) =>
              onEntityChange({
                tags: value
                  .split(",")
                  .map((tag) => tag.trim())
                  .filter(Boolean),
              })
            }
          />
        </InspectorGroup>

        <InspectorGroup title="Transform">
          <VectorRow
            label="Position"
            values={[entity.transform.x, entity.transform.y, entity.transform.z]}
            onChange={([x, y, z]) => onTransformChange({ x, y, z })}
          />
          <VectorRow
            label="Rotation"
            values={[entity.transform.rotationX, entity.transform.rotationY, entity.transform.rotationZ]}
            onChange={([rotationX, rotationY, rotationZ]) =>
              onTransformChange({ rotationX, rotationY, rotationZ })
            }
          />
          <VectorRow
            label="Scale"
            values={[entity.transform.scaleX, entity.transform.scaleY, entity.transform.scaleZ]}
            onChange={([scaleX, scaleY, scaleZ]) => onTransformChange({ scaleX, scaleY, scaleZ })}
            step={0.1}
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
          <div className="add-component-row">
            <select
              className="component-input"
              value=""
              onChange={(event) => {
                if (!event.target.value) return;
                onEntityChange({
                  components: [...entity.components, createComponentFromSchema(event.target.value)],
                });
              }}
            >
              <option value="">+ Add Component</option>
              {Object.entries(borealisComponents())
                .filter(([kind]) => !entity.components.some((c) => c.kind === kind))
                .map(([kind, schema]) => (
                  <option key={kind} value={kind}>
                    {kind} - {schema.description}
                  </option>
                ))}
            </select>
          </div>
        </InspectorGroup>

        <InspectorGroup title="Script">
          <label className="field-label" htmlFor="attached-script">
            Attached script
          </label>
          <select id="attached-script" value={attachedScript} onChange={(event) => onAttachScript(event.target.value)}>
            <option value="">None</option>
            {scripts.map((script) => (
              <option key={script.path} value={script.path}>
                {script.name}
              </option>
            ))}
          </select>
        </InspectorGroup>

        <SceneSettingsPanel assets={assets} scene={scene} scripts={scripts} onSceneChange={onSceneChange} />
      </div>
    </aside>
  );
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

  return (
    <InspectorGroup title="Scene Settings" noPadding>
      {entries.map(([key, schema]) => {
        const section = sceneSettingSection(key);
        const values = sceneSettingValues(scene, key);
        const pseudoComponent: SceneComponent = { kind: key, properties: values };

        return (
          <div className="component-card" key={key}>
            <div className="component-card-header">
              <Settings2 size={15} strokeWidth={ICON_STROKE} />
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
                      scripts={scripts}
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
