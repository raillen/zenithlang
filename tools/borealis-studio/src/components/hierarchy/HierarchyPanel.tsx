import { ChevronDown, ChevronRight, FolderTree, MoreHorizontal, Search } from "lucide-react";
import React, { useMemo } from "react";
import { ICON_STROKE } from "../../constants";
import type { HierarchyTreeNode, SceneEntity } from "../../types";
import { entityIcon } from "../../utils/entity";
import { PanelHeader } from "../shared";

function buildHierarchy(entities: SceneEntity[]): HierarchyTreeNode[] {
  const entityIds = new Set(entities.map((entity) => entity.id));
  const childrenByParent = new Map<string | undefined, SceneEntity[]>();
  for (const entity of entities) {
    const parent = normalizedParentId(entity.parent, entityIds, entity.id);
    const siblings = childrenByParent.get(parent) ?? [];
    siblings.push(entity);
    childrenByParent.set(parent, siblings);
  }

  for (const siblings of childrenByParent.values()) {
    siblings.sort((a, b) => a.name.localeCompare(b.name));
  }

  const walk = (parentId: string | undefined, depth: number): HierarchyTreeNode[] => {
    return (childrenByParent.get(parentId) ?? []).map((entity) => {
      const children = walk(entity.id, depth + 1);
      return {
        id: entity.id,
        name: entity.name,
        layer: entity.layer,
        depth,
        parentId: normalizedParentId(entity.parent, entityIds, entity.id),
        children,
        hasChildren: children.length > 0,
      };
    });
  };

  return walk(undefined, 0);
}

function normalizedParentId(parent: unknown, entityIds: Set<string>, entityId: string): string | undefined {
  if (typeof parent !== "string") return undefined;
  const normalized = parent.trim();
  if (!normalized || normalized === entityId || !entityIds.has(normalized)) return undefined;
  return normalized;
}

function filterHierarchy(nodes: HierarchyTreeNode[], query: string): HierarchyTreeNode[] {
  const normalized = query.trim().toLowerCase();
  if (!normalized) return nodes;

  return nodes
    .map((node) => {
      const children = filterHierarchy(node.children, query);
      const selfMatch = node.name.toLowerCase().includes(normalized) || node.layer.toLowerCase().includes(normalized);
      if (!selfMatch && children.length === 0) return null;
      return {
        ...node,
        children,
      };
    })
    .filter((node): node is HierarchyTreeNode => node !== null);
}

export function HierarchyPanel({
  entities,
  expandedEntityIds,
  sceneName,
  sceneRootSelected,
  search,
  selectedEntityId,
  onReparentEntity,
  onSearchChange,
  onSelectEntity,
  onSelectSceneRoot,
  onToggleExpanded,
}: {
  entities: SceneEntity[];
  expandedEntityIds: string[];
  sceneName: string;
  sceneRootSelected: boolean;
  search: string;
  selectedEntityId: string;
  onReparentEntity: (entityId: string, newParentId?: string) => void;
  onSearchChange: (value: string) => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onSelectSceneRoot: () => void;
  onToggleExpanded: (entityId: string) => void;
}) {
  const entityById = useMemo(() => new Map(entities.map((entity) => [entity.id, entity])), [entities]);
  const hierarchy = useMemo(() => filterHierarchy(buildHierarchy(entities), search), [entities, search]);

  return (
    <section className="panel-shell hierarchy-panel">
      <PanelHeader icon={<FolderTree size={14} strokeWidth={ICON_STROKE} />} meta="Scene" title="Hierarchy">
        <button className="panel-icon-button" title="Hierarchy actions" type="button">
          <MoreHorizontal size={14} strokeWidth={ICON_STROKE} />
        </button>
      </PanelHeader>
      <label className="panel-search">
        <Search size={14} strokeWidth={ICON_STROKE} />
        <input onChange={(event) => onSearchChange(event.target.value)} placeholder="Search hierarchy" value={search} />
      </label>
      <div className="hierarchy-tree">
        <button
          className={`hierarchy-row hierarchy-root ${sceneRootSelected ? "selected" : ""}`}
          onClick={onSelectSceneRoot}
          onDragOver={(event) => event.preventDefault()}
          onDrop={(event) => {
            const draggedId = event.dataTransfer.getData("application/x-borealis-entity-id");
            if (draggedId) onReparentEntity(draggedId, undefined);
          }}
          type="button"
        >
          <ChevronDown size={13} strokeWidth={ICON_STROKE} />
          <strong>{sceneName}</strong>
          <small>scene</small>
        </button>
        {hierarchy.map((node) => (
          <HierarchyTree
            entityById={entityById}
            expandedEntityIds={expandedEntityIds}
            key={node.id}
            node={node}
            selectedEntityId={selectedEntityId}
            onReparentEntity={onReparentEntity}
            onSelectEntity={onSelectEntity}
            onToggleExpanded={onToggleExpanded}
          />
        ))}
      </div>
    </section>
  );
}

export function HierarchyTree({
  entityById,
  expandedEntityIds,
  node,
  selectedEntityId,
  onReparentEntity,
  onSelectEntity,
  onToggleExpanded,
}: {
  entityById: Map<string, SceneEntity>;
  expandedEntityIds: string[];
  node: HierarchyTreeNode;
  selectedEntityId: string;
  onReparentEntity: (entityId: string, newParentId?: string) => void;
  onSelectEntity: (entity: SceneEntity) => void;
  onToggleExpanded: (entityId: string) => void;
}) {
  const expanded = expandedEntityIds.includes(node.id);
  const entity = entityById.get(node.id);
  if (!entity) return null;

  return (
    <div className="hierarchy-branch">
      <button
        className={`hierarchy-row ${selectedEntityId === node.id ? "selected" : ""}`}
        draggable
        onClick={() => onSelectEntity(entity)}
        onDragStart={(event) => {
          event.dataTransfer.setData("application/x-borealis-entity-id", node.id);
        }}
        onDragOver={(event) => event.preventDefault()}
        onDrop={(event) => {
          event.preventDefault();
          const draggedId = event.dataTransfer.getData("application/x-borealis-entity-id");
          if (draggedId && draggedId !== node.id) {
            onReparentEntity(draggedId, node.id);
          }
        }}
        style={{ paddingLeft: `${12 + node.depth * 18}px` }}
        type="button"
      >
        <span
          className="hierarchy-expander"
          onClick={(event) => {
            event.preventDefault();
            event.stopPropagation();
            if (node.hasChildren) onToggleExpanded(node.id);
          }}
        >
          {node.hasChildren ? (
            expanded ? <ChevronDown size={13} strokeWidth={ICON_STROKE} /> : <ChevronRight size={13} strokeWidth={ICON_STROKE} />
          ) : (
            <span className="hierarchy-expander-spacer" />
          )}
        </span>
        <span className="hierarchy-icon">{entityIcon(entity)}</span>
        <span className="hierarchy-name">{node.name}</span>
        <small>{node.layer}</small>
      </button>
      {expanded && node.children.length > 0
        ? node.children.map((child) => (
            <HierarchyTree
              entityById={entityById}
              expandedEntityIds={expandedEntityIds}
              key={child.id}
              node={child}
              selectedEntityId={selectedEntityId}
              onReparentEntity={onReparentEntity}
              onSelectEntity={onSelectEntity}
              onToggleExpanded={onToggleExpanded}
            />
          ))
        : null}
    </div>
  );
}
