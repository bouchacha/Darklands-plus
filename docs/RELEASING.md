# Releasing Darklands+

A release is a Windows zip that holds only this project's own code and the tools that make the
game's code on the player's machine (see `port/README.md`, "Releases"). It is built on GitHub.

## Once: put the repository on GitHub

1. Create an empty repository on github.com (no README, licence or .gitignore; this repository
   has its own).
2. Add a licence of your choice as `LICENSE` at the top of this repository and commit it. The
   packager puts it into the zip with the third-party notices. The third-party parts keep their
   own licences: Dear ImGui (MIT), Nuked OPL3 (LGPL 2.1+), stb (public domain / MIT), the fonts
   (SIL OFL 1.1), SDL2 (zlib), TinyCC (LGPL 2.1), Python (PSF), capstone (BSD).
3. Push:

   ```
   git remote add origin https://github.com/<you>/<repository>.git
   git push -u origin HEAD
   ```

Never commit anything from `build/`, game files, or a `darklands_game.dll`. `.gitignore` keeps
`build/` out.

## Each release

1. Make a test zip: on GitHub, open **Actions > Windows release > Run workflow**. When the run
   finishes, its page has the zip under **Artifacts**.
2. Try it on Windows: unpack the zip into a copy of the Darklands folder, start
   `darklands.exe`, and let the first start finish (a few minutes). Then play for a while:
   Quickstart, a city, travel, a battle, saving and loading.
3. Publish it with a tag. GitHub then builds the zip again and attaches it to a new release:

   ```
   git tag v1.0
   git push origin v1.0
   ```

   The release's text is `docs/release_notes.md` (what Darklands+ is, what it needs, how to
   install it, what it adds); update it before tagging, or edit the release on GitHub afterwards
   to say what changed since the last one.

## If the workflow fails

The log of the failed step says why. The steps are, in order: SDL2 for mingw-w64 (downloaded
from the SDL releases on GitHub), the program (`make -C port SPLIT=1 TARGET=win64`), TinyCC
(`port/tools/build_tcc_win.sh`, from the TinyCC repository on GitHub), Python for Windows
(python-build-standalone's `install_only` build, from its GitHub releases) and capstone (PyPI),
and the zip (`port/tools/package.py`). The versions and the files' checksums are set at the top
of `.github/workflows/release.yml`; a download whose checksum differs stops the run.

`package.py` can also take python.org's "Windows embeddable package" (`--python-zip`) instead of
a whole Python (`--python-dir`, trimmed of what the translator does not need); the release uses
the latter because that is what the first start was tested with.

## What players need

Their own Darklands 483.07 (GOG) and Windows 10 or 11, 64-bit. On Linux and Steam Deck the
Windows zip runs under Wine / Proton. The players' README in the zip is
`port/tools/release_readme.txt`.
