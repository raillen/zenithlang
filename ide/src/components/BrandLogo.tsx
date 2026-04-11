type BrandVariant = "icon" | "lockup" | "wordmark" | "vertical";

type BrandLogoProps = {
  variant?: BrandVariant;
  className?: string;
  alt?: string;
};

const BRAND_ASSETS: Record<BrandVariant, string> = {
  icon: "/logo-only.svg",
  lockup: "/logo-with-text.svg",
  wordmark: "/logo-text-only.svg",
  vertical: "/logo-with-text-vertical.svg",
};

export function BrandLogo({
  variant = "lockup",
  className = "",
  alt,
}: BrandLogoProps) {
  return (
    <img
      src={BRAND_ASSETS[variant]}
      alt={
        alt ??
        (variant === "icon"
          ? "Zenith logo"
          : variant === "wordmark"
            ? "Zenith wordmark"
            : variant === "vertical"
              ? "Zenith vertical logo"
              : "Zenith logo with text")
      }
      className={`select-none ${className}`.trim()}
      draggable={false}
    />
  );
}
