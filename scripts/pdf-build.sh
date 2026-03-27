#!/bin/bash

set -euo pipefail

./scripts/render-svg-pngs.sh

if [[ "${REVIEW_FORCE_LOCAL:-}" == "1" ]]; then
  exec grunt pdf
fi

if command -v docker >/dev/null 2>&1; then
  echo "Running PDF build in Docker." >&2
  exec ./build-in-docker.sh
fi

echo "docker is required for PDF build." >&2
exit 1
