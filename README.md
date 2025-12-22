# FIDS — Fullscreen Interactive Directory Shell

FIDS is a two-pane, fullscreen directory shell for VT100/ANSI terminals on UNIX-like systems. It provides fast, keyboard-driven file browsing, tagging, and command execution without leaving the terminal. The project dates back to the early 1980s and has been continually refined and ported across UNIX variants.

> Release: 2.9.87 • Platforms: UNIX, Linux, QNX, SCO, SunOS, ULTRIX, DEC-UNIX • Large-file support available on Linux

## Overview
- Two synchronized directory panels (left/right) with split-screen toggle
- VT100/ANSI graphics and attributes, works in plain terminals
- Tagging and bulk operations on files/directories
- Inline command editing and execution (e.g., `cp`, `mv`, `rm`, `chmod`, `chown`)
- Configurable PF keys and Ctrl keys via `.fids.pf_keys`
- Custom per-filetype execution via `.fids.ex_cmds`
- Persist and restore UI settings (`.fids.ss`)
- Help pages built-in (ESC `?`)

Background and credits can be found in the historical note in [src/README](src/README).

## Build
Choose a platform-specific makefile from `src/`.

On Linux (with large-file support):

```bash
cd src
make -f makefile.LinuxLFS
```

On Linux (2 GB file size target):

```bash
cd src
make -f makefile.Linux2GB
```

Other platforms have dedicated makefiles and flags inside `src/makefile.*`. The build produces the `fids` binary.

### Install
The provided `install` target copies useful helpers alongside `fids`:

```bash
cd src
make -f makefile.LinuxLFS install
```

This installs to `/usr/local/bin` (overwriting a previous `fids` while saving it as `fids.old`), and includes:
- `fids` — main application
- `fren` — helper script for batch rename/copy/link
- `.fids.ex_cmds` — per-filetype execution helper
- `.fids.pf_keys` — sample PF/Ctrl key mappings

You may also copy these into your `$PATH` manually if preferred.

## Run
Start `fids` in a terminal with VT100/ANSI capabilities:

```bash
fids
```

Minimum terminal size is roughly 15 lines × 52 columns. Press ESC `?` anytime for help pages.

## Key Bindings (Essentials)
FIDS offers rich bindings; here are the most used defaults (from the built-in help):

- Panel & navigation:
  - ESC `/`: Enter command mode (or `cd` if cursor on directory)
  - ESC `#`: Toggle split screen (1/2 panels)
  - ESC `P` / `N`: Page up/down; ESC `I`: switch side
  - ESC `<` / `>`: Move to first/last file in list
- Tagging & actions:
  - ESC `@`: Tag current item; ESC `T`/`U`: tag/untag all; ESC `*`: toggle all
  - ESC `V`: Quick view; ESC `R`: Remove item; ESC `X`: Execute selected (or `cd` on dir)
- Command line helpers:
  - ESC `C`/`M`: Insert `cp`/`mv` with operands
  - ESC `O`: Insert filename from the other pane
  - ESC `1`…`9`: Run PF function keys
  - ESC `s` / `r`: Store/restore current directory
  - ESC `?`: Show help pages

Full listings for directory and command-line modes are embedded in help; see [src/fids.hlp](src/fids.hlp).

## Command Mode & Substitution
Edit the command line directly and execute without leaving FIDS. Special characters:
- `@`: Loop over tagged files (e.g., `mv @ /tmp`)
- `$`: Substitute environment variable (e.g., `cp @ $HOME/bin`)
- `%`: Substitute command output (e.g., `cd %whereis ping%`)
- `=`: Set environment variable (e.g., `DISPLAY=%uname -n%:0.0`)

You can customize the substitution characters via `fids_set` (see help).

## Configuration
- PF/Ctrl keys: Customize using `.fids.pf_keys`. Example entries include aliases for copy/move or custom helpers. See [src/.fids.pf_keys](src/.fids.pf_keys).
- Per-filetype execution: `.fids.ex_cmds` lets you define what happens when executing certain files (e.g., open images with `xv`, run `gunzip` on `.gz`). See [src/.fids.ex_cmds](src/.fids.ex_cmds).
- Saved settings: FIDS can persist UI state in `.fids.ss` and uses `PS1` prefixed with `[fids]` for shell prompts while active.

## Helper: `fren`
`fren` is a shell helper to batch rename/copy/link with placeholders:

```bash
fren mv *.c %.cold        # rename .c files to .cold
fren cp myprog.* newprog.+ # copy preserving extensions
```

See [src/fren](src/fren) for usage and options like delimiter, interactive, and verbose modes.

## Platform Notes
- Terminal capabilities and colors are detected (VT100/ANSI); attributes are defined in [src/fids.h](src/fids.h).
- Ports exist for QNX4/QNX6 and others; see `makefile.*` options like `-D_ANSI_COLOR`, `-D_FDRNFS`, `-D_IRSDEL`, and platform-specific libraries.
- Linux large-file support uses `-D_FILE_OFFSET_BITS=64` (see [src/makefile.LinuxLFS](src/makefile.LinuxLFS)).

## License
A permissive license is embedded in the source headers (e.g., [src/fids.c](src/fids.c)), permitting use, copy, modification, and distribution without fee, provided copyright and permission notices are retained and the author’s name is not used for endorsement without prior written permission.

## Credits
FIDS was created by BeNo (BeNoSoft), with contributions and ports by many, including MB, HDS and PST. See the historical note in [src/README](src/README).

## Troubleshooting
- If execution of selected files does nothing, ensure `.fids.ex_cmds` is in your `$PATH` and executable.
- If PF keys don’t apply, load them via ESC `l` or check that `.fids.pf_keys` is discoverable in `cwd`, `$HOME`, or `$PATH`.
- For NFS or networked filesystems, force rereads via build flags like `-D_FDRNFS`.

## Quick Start
```bash
# Build on Linux with large-file support
cd src
make -f makefile.LinuxLFS

# Run
./fids

# Help inside FIDS
# Press ESC then '?'
```
