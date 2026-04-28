import React from "react";

export function TagLayerRow({
  layer,
  layers,
  tags,
  onLayerChange,
  onTagsChange,
}: {
  layer: string;
  layers: string[];
  tags: string[];
  onLayerChange: (value: string) => void;
  onTagsChange: (value: string[]) => void;
}) {
  return (
    <div className="tag-layer-row">
      <label>
        <span>Tag</span>
        <input
          onChange={(event) =>
            onTagsChange(
              event.target.value
                .split(",")
                .map((tag) => tag.trim())
                .filter(Boolean),
            )
          }
          value={tags.join(", ")}
        />
      </label>
      <label>
        <span>Layer</span>
        <select onChange={(event) => onLayerChange(event.target.value)} value={layer}>
          {layers.map((item) => (
            <option key={item} value={item}>
              {item}
            </option>
          ))}
        </select>
      </label>
    </div>
  );
}
