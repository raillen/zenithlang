import {
  AppWindow,
  BugBeetle,
  Files,
  GearSix,
  GitBranch,
  MagnifyingGlass,
  type Icon,
} from '@phosphor-icons/react';
import { type SidebarView, useProjectStore } from '../store/useProjectStore';
import { uiMarker } from '../utils/debugSelectors';

const activityItems: Array<{ id: SidebarView; label: string; icon: Icon }> = [
  { id: 'explorer', label: 'Explorer', icon: Files },
  { id: 'search', label: 'Search', icon: MagnifyingGlass },
  { id: 'sourceControl', label: 'Source Control', icon: GitBranch },
  { id: 'run', label: 'Run', icon: BugBeetle },
  { id: 'extensions', label: 'Tools', icon: AppWindow },
];

interface ActivityBarProps {
  activeView: SidebarView;
  onViewChange: (view: SidebarView) => void;
  onOpenSettings: () => void;
}

export function ActivityBar({ activeView, onViewChange, onOpenSettings }: ActivityBarProps) {
  const { focusMode } = useProjectStore();

  return (
    <aside
      className="flex h-full w-[52px] shrink-0 flex-col items-center border-r pt-2"
      style={{ borderColor: 'var(--border)', background: 'color-mix(in srgb, var(--ide-sidebar) 82%, white 18%)' }}
      {...uiMarker('activity-bar', { activeView })}
    >
      <div className="h-3 w-full shrink-0" {...uiMarker('activity-bar-drag-region')} />

      <div className="mt-1 flex w-full items-center justify-center px-2" {...uiMarker('activity-bar-brand')}>
        <div
          className="flex h-8 w-8 items-center justify-center rounded-[12px] border"
          style={{
            borderColor: 'var(--border)',
            background: 'color-mix(in srgb, var(--ide-panel) 88%, white 12%)',
          }}
          title="Zenith"
          aria-label="Zenith"
          {...uiMarker('activity-bar-logo')}
        >
          <img src="/logo-only.svg" alt="Zenith" className="h-4 w-4 object-contain" {...uiMarker('activity-bar-logo-image')} />
        </div>
      </div>

      <div className="mt-3 flex w-full flex-1 flex-col items-center gap-2" {...uiMarker('activity-bar-nav')}>
        {activityItems.map(({ id, label, icon: IconShape }) => {
          const isActive = activeView === id;

          return (
            <button
              key={id}
              type="button"
              title={label}
              aria-label={label}
              aria-pressed={isActive}
              onClick={() => onViewChange(id)}
              className="group relative flex h-10 w-full items-center justify-center transition"
              style={{
                background: isActive ? 'var(--accent-soft)' : 'transparent',
                color: isActive ? 'var(--accent)' : 'var(--text-tertiary)',
              }}
              {...uiMarker('activity-bar-item', { view: id, active: isActive })}
            >
              {isActive && (
                <span
                  className="absolute left-0 top-1/2 h-5 w-[2px] -translate-y-1/2 rounded-full"
                  style={{ background: 'var(--accent)' }}
                  {...uiMarker('activity-bar-item-indicator', { view: id })}
                />
              )}
              <IconShape size={16} weight={isActive ? 'fill' : 'regular'} />
              <span
                className={`activity-tooltip pointer-events-none absolute left-full top-1/2 z-20 ml-3 -translate-y-1/2 whitespace-nowrap rounded-full border px-2 py-1 text-[11px] opacity-0 shadow-lg transition group-hover:opacity-100 ${
                  focusMode ? 'hidden' : ''
                }`}
                style={{
                  borderColor: 'var(--border)',
                  background: 'color-mix(in srgb, var(--ide-panel) 92%, white 8%)',
                  color: 'var(--text-secondary)',
                  boxShadow: 'var(--panel-shadow)',
                }}
                {...uiMarker('activity-bar-tooltip', { view: id })}
              >
                {label}
              </span>
            </button>
          );
        })}
      </div>

      <div className="mb-2 flex w-full flex-col items-center" {...uiMarker('activity-bar-footer')}>
        <button
          type="button"
          title="Settings"
          aria-label="Settings"
          onClick={onOpenSettings}
          className="flex h-10 w-full items-center justify-center transition"
          style={{ color: 'var(--text-tertiary)' }}
          {...uiMarker('activity-bar-settings')}
        >
          <GearSix size={16} weight="regular" />
        </button>
      </div>
    </aside>
  );
}
