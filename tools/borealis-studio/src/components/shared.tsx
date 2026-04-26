import { ChevronDown } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../constants";
import type { ViewMode } from "../types";

export function StatusPill({ children, tone = "neutral" }: { children: React.ReactNode; tone?: "neutral" | "good" | "warn" }) {
  return <span className={`status-pill ${tone}`}>{children}</span>;
}

export function ToolButton({
  active,
  children,
  disabled,
  label,
  onClick,
}: {
  active?: boolean;
  children: React.ReactNode;
  disabled?: boolean;
  label: string;
  onClick?: () => void;
}) {
  return (
    <button
      aria-pressed={active}
      className={`tool-button ${active ? "active" : ""}`}
      disabled={disabled}
      onClick={onClick}
      title={label}
    >
      {children}
    </button>
  );
}

export function PanelHeader({
  children,
  icon,
  meta,
  title,
}: {
  children?: React.ReactNode;
  icon: React.ReactNode;
  meta: string;
  title: string;
}) {
  return (
    <div className="panel-header">
      <div>
        {icon}
        <strong>{title}</strong>
        <span>{meta}</span>
      </div>
      {children}
    </div>
  );
}

export function SectionTitle({ label }: { label: string }) {
  return (
    <div className="section-title">
      <ChevronDown size={11} strokeWidth={ICON_STROKE} />
      <span>{label}</span>
    </div>
  );
}

export function InspectorGroup({ children, title, noPadding }: { children: React.ReactNode; title: string; noPadding?: boolean }) {
  return (
    <section className="inspector-group">
      <div className="inspector-group-header">
        <ChevronDown size={11} strokeWidth={ICON_STROKE} />
        <h3>{title}</h3>
      </div>
      <div className={`inspector-group-content ${noPadding ? "no-padding" : ""}`}>
        {children}
      </div>
    </section>
  );
}

export function TextField({
  label,
  onChange,
  value,
}: {
  label: string;
  onChange: (value: string) => void;
  value: string;
}) {
  return (
    <label className="text-field">
      <span>{label}</span>
      <input value={value} onChange={(event) => onChange(event.target.value)} />
    </label>
  );
}

export function VectorRow({
  label,
  onChange,
  step = 1,
  values,
}: {
  label: string;
  onChange: (values: [number, number, number]) => void;
  step?: number;
  values: [number, number, number];
}) {
  return (
    <div className="vector-row">
      <span>{label}</span>
      {(["X", "Y", "Z"] as const).map((axis, index) => (
        <label key={axis}>
          <small>{axis}</small>
          <input
            step={step}
            type="number"
            value={Number(values[index].toFixed(2))}
            onChange={(event) => {
              const next = [...values] as [number, number, number];
              next[index] = Number(event.target.value);
              onChange(next);
            }}
          />
        </label>
      ))}
    </div>
  );
}

export function Segmented<T extends string>({
  onChange,
  tabs,
  value,
}: {
  onChange: (value: T) => void;
  tabs: T[];
  value: T;
}) {
  return (
    <div className="segmented">
      {tabs.map((tab) => (
        <button className={value === tab ? "selected" : ""} key={tab} onClick={() => onChange(tab)}>
          {tab}
        </button>
      ))}
    </div>
  );
}

export function DockTab({
  active,
  children,
  icon,
  onClick,
}: {
  active: boolean;
  children: React.ReactNode;
  icon: React.ReactNode;
  onClick: () => void;
}) {
  return (
    <button className={active ? "selected" : ""} onClick={onClick}>
      {icon}
      {children}
    </button>
  );
}

export function ObjectGizmo({ viewMode }: { viewMode: ViewMode }) {
  return (
    <div className="object-gizmo" aria-hidden="true">
      <span className="gizmo-x" />
      <span className="gizmo-y" />
      {viewMode === "3d" ? <span className="gizmo-z" /> : null}
    </div>
  );
}
