#!/bin/bash

set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  exit 0
fi

if ! command -v qlmanage >/dev/null 2>&1 || ! command -v sips >/dev/null 2>&1; then
  echo "Skipping SVG to PNG refresh: qlmanage or sips is unavailable." >&2
  exit 0
fi

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
images_dir="${repo_root}/articles/images"
thumb_size="${SVG_RENDER_SIZE:-1600}"
tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

shopt -s nullglob

for svg in "${images_dir}"/*.svg; do
  png="${svg%.svg}.png"
  basename="$(basename "$svg")"
  ql_png="${tmp_dir}/${basename}.png"

  read -r svg_width svg_height < <(
    sed -n 's/.*width="\([0-9.]*\)".*height="\([0-9.]*\)".*/\1 \2/p' "$svg" | head -n 1
  )

  if [[ -z "${svg_width:-}" || -z "${svg_height:-}" ]]; then
    echo "Skipping ${basename}: width/height not found." >&2
    continue
  fi

  qlmanage -t -s "${thumb_size}" -o "${tmp_dir}" "${svg}" >/dev/null 2>&1

  if (( $(awk "BEGIN { print (${svg_width} >= ${svg_height}) ? 1 : 0 }") )); then
    crop_width="${thumb_size}"
    crop_height="$(awk "BEGIN { printf \"%d\", (${thumb_size} * ${svg_height} / ${svg_width}) + 0.5 }")"
    sips -c "${crop_height}" "${crop_width}" "${ql_png}" --out "${png}" >/dev/null
  else
    crop_height="${thumb_size}"
    crop_width="$(awk "BEGIN { printf \"%d\", (${thumb_size} * ${svg_width} / ${svg_height}) + 0.5 }")"
    sips -c "${crop_height}" "${crop_width}" "${ql_png}" --out "${png}" >/dev/null
  fi
done
