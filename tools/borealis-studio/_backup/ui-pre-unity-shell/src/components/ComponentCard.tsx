import { Component, X } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../constants";
import type { ComponentFieldSchema, ProjectAsset, SceneComponent, ScriptDocument } from "../types";
import {
  componentSchema,
  componentValue,
  setComponentValue,
} from "../borealisCatalog";
import {
  assetOptionsForComponentField,
  componentInputValue,
  formatComponentValue,
  includeCurrentOption,
} from "../utils/component";

export function ComponentCard({
  assets,
  component,
  scripts,
  onChange,
  onRemove,
}: {
  assets: ProjectAsset[];
  component: SceneComponent;
  scripts: ScriptDocument[];
  onChange: (component: SceneComponent) => void;
  onRemove?: () => void;
}) {
  const schema = componentSchema(component);
  const knownKeys = new Set(["kind", "type", ...schema.fields.map((field) => field.key)]);
  const unknownEntries = Object.entries(component.properties ?? {}).filter(([key]) => !knownKeys.has(key));

  return (
    <div className="component-card">
      <div className="component-card-header">
        <Component size={15} strokeWidth={ICON_STROKE} />
        <strong>{component.kind}</strong>
        {onRemove && (
          <button className="remove-btn" onClick={onRemove} title="Remove Component">
            <X size={14} strokeWidth={ICON_STROKE} />
          </button>
        )}
      </div>
      <div className="component-card-body">
        {schema.fields.length > 0 ? (
          <div className="component-fields">
            {schema.fields.map((field) => (
              <div className="component-field" key={field.key}>
                <span>{field.label}</span>
                <ComponentFieldControl
                  assets={assets}
                  component={component}
                  field={field}
                  scripts={scripts}
                  onChange={(value) => onChange(setComponentValue(component, field.key, value))}
                />
              </div>
            ))}
          </div>
        ) : null}
        {unknownEntries.length > 0 ? (
          <div className="component-fields unknown">
            {unknownEntries.map(([key, value]) => (
              <div className="component-field" key={key}>
                <span>{key}</span>
                <code>{formatComponentValue(value)}</code>
              </div>
            ))}
          </div>
        ) : null}
      </div>
    </div>
  );
}

export function ComponentFieldControl({
  assets,
  component,
  field,
  scripts,
  onChange,
}: {
  assets: ProjectAsset[];
  component: SceneComponent;
  field: ComponentFieldSchema;
  scripts: ScriptDocument[];
  onChange: (value: unknown) => void;
}) {
  const value = componentValue(component, field.key);

  if (field.kind === "boolean") {
    return (
      <input
        checked={value === true}
        className="component-check"
        onChange={(event) => onChange(event.target.checked)}
        type="checkbox"
      />
    );
  }

  if (field.kind === "number") {
    return (
      <input
        className="component-input"
        max={field.max}
        min={field.min}
        onChange={(event) => onChange(event.target.value === "" ? undefined : Number(event.target.value))}
        step={field.step ?? "any"}
        type="number"
        value={componentInputValue(value)}
      />
    );
  }

  if (field.kind === "color") {
    const currentValue = typeof value === "string" && value.startsWith("#") ? value : "#ffffff";
    return (
      <input
        className="component-input component-color-input"
        onChange={(event) => onChange(event.target.value)}
        type="color"
        value={currentValue}
      />
    );
  }

  if (field.kind === "select") {
    return (
      <select className="component-input" onChange={(event) => onChange(event.target.value)} value={String(value ?? "")}>
        <option value="">None</option>
        {(field.options ?? []).map((option) => (
          <option key={option} value={option}>
            {option}
          </option>
        ))}
      </select>
    );
  }

  if (field.kind === "asset") {
    const currentValue = String(value ?? "");
    const options = includeCurrentOption(assetOptionsForComponentField(component, field, assets, scripts), currentValue);
    return (
      <select
        className="component-input"
        onChange={(event) => onChange(event.target.value || undefined)}
        value={currentValue}
      >
        <option value="">None</option>
        {options.map((option) => (
          <option key={option} value={option}>
            {option}
          </option>
        ))}
      </select>
    );
  }

  return (
    <input
      className="component-input"
      onChange={(event) => onChange(event.target.value)}
      value={componentInputValue(value)}
    />
  );
}
