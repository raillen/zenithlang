type BrandVariant = "icon" | "lockup";

type BrandLogoProps = {
  variant?: BrandVariant;
  className?: string;
  alt?: string;
};

const BRAND_ASSETS: Record<BrandVariant, string> = {
  icon: "/logo-only.svg",
  lockup: "/logo-with-text.svg",
};

export function BrandLogo({
  variant = "lockup",
  className = "",
  alt,
}: BrandLogoProps) {
  return (
    <img
      src={BRAND_ASSETS[variant]}
      alt={alt ?? (variant === "icon" ? "Zenith logo" : "Zenith logo with text")}
      className={`select-none ${className}`.trim()}
      draggable={false}
    />
  );
}
