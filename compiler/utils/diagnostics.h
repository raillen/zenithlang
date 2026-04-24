/* Thin compatibility wrapper.
 *
 * This header used to host a duplicated copy of the diagnostics public API
 * that drifted out of sync with the canonical header in
 * `compiler/semantic/diagnostics/diagnostics.h`. Because both headers share
 * the same include guard, the effective contents depended on include order
 * and caused silent divergence of the `zt_diag_code` enum.
 *
 * The canonical contract now lives in `compiler/semantic/diagnostics/diagnostics.h`.
 * This file re-exports it so that historical consumers (notably
 * `compiler/utils/l10n.h`) keep compiling without changes.
 */

#include "compiler/semantic/diagnostics/diagnostics.h"


