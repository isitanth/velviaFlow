# velviaFlow CLI

**Author:** Anthony Chambet  
**Target:** Fujifilm SD cards (X-T series, macOS arm64)  
**Purpose:** Terminal-first workflow utility for photographers. Creates a dedicated workspace for each run to ensure safe handling of photos/videos, with backup, stats, and cleanup features.  
**Tested with:** Fujifilm X-T5

---

## Structure

```
velviaflow-c/
├── Makefile             # Build rules (clang, C17, macOS arm64)
├── LICENSE              # License information
├── README.md            # Project documentation
├── include/             # Public headers (cli.h, fsutil.h, media.h…)
├── src/                 # Implementation files
│   ├── main.c           # Entry point
│   ├── cli.c            # CLI parser
│   ├── ops.c            # Operations (backup, cleanup…)
│   └── workspace.c      # Workspace management
└── workdir/             # Auto-created runtime workspace
    ├── inbox/           # Safe copy of SD files
    ├── velviaSort/      # Sorted backups YYYY_MM/{rawPictures,rawRushs}
    └── .velviaflow/     # Internal state/manifest
```

---

## Usage

1. Insert your Fujifilm SD card (must contain `DCIM/`).  
2. Run the CLI tool with the appropriate option:

```bash
velviaflow -check       # Detect SD card
velviaflow -start       # Initialize workspace & copy files
velviaflow -stats       # Show totals (photos, videos, size)
velviaflow -backup      # Sort & copy files with manifest
velviaflow -cleanup     # Safely remove inbox files
velviaflow -v           # Version info
```

> Each run creates or reuses a **workspace** under `workdir/` to guarantee **safe-by-default operations** and prevent accidental data loss.

---

## Install & Build

```bash
make
sudo make install   # Installs velviaflow into /usr/local/bin/
```

Dependencies:  
- **clang** (C17)  
- macOS ARM64 (only M1 Pro tested)

---

## Roadmap

- Extended Fujifilm RAW/Video format detection.  
- Automated pipelines for post-processing (renaming, metadata, conversion).  
- Optional integration with external photo libraries / cloud backup.  
- CI testing, portability to Linux.

---

## Context

- Designed for photographers who want **fast, reliable, and safe SD card ingestion** directly from the terminal.  
- The **workspace model** ensures that original media is never touched until explicitly backed up and confirmed via manifest.  
- Inspired by the principle: *“Automate safely. Process your picture with ease.”*

---
