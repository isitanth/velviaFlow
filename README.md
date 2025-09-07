# velviaflow

Fujifilm SD photo workflow utility for macOS (arm64). Terminal-first, safe-by-default.

## Features
- `-start`: mandatory workspace bootstrap (`inbox/`, `velviaSort/`, `.velviaflow/`)
- `-check`: detect SD-like volumes (DCIM + media)
- `-stats`: totals over workspace inbox
- `-backup`: inbox → `velviaSort/YYYY_MM/{rawPictures,rawRushs}` (+ manifest)
- `-cleanup`: delete sources listed in a manifest (safety checks by default)

## Install & Build
```bash
make
sudo make install  # /usr/local/bin/velviaflow