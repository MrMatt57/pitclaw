"""Pre-build: reconcile LovyanGFX with our LVGL 9.x install.

LovyanGFX bundles its own copy of LVGL types and font tables for users
without real LVGL. With both libraries linked we get include-shadowing of
<lvgl.h> and duplicate `lv_font_*` symbols at link.

This script:
  1. Removes LovyanGFX's `src/lvgl.h` so our `<lvgl.h>` finds real LVGL.
  2. Replaces `src/lgfx/v1/lvgl.h` with a stub that pulls in real LVGL and
     defines the `LV_FONT_GLYPH_FORMAT_A*_ALIGNED` enum values that
     LovyanGFX uses internally but LVGL 9.4 does not export.
  3. Deletes the vendored `lv_font/` types (would re-define LVGL symbols).
  4. Deletes bundled `Fonts/lvgl/*.c` font tables (duplicate symbols with
     LVGL's montserrat fonts) and replaces `Fonts/lvgl/lvgl.h` with a stub.
  5. Disables the `LVGLfont lv_font_*` wrapper instances and their externs
     in `lgfx_fonts.cpp` / `lgfx_fonts.hpp`. The wrapper class itself
     stays and compiles; the impls are unreferenced and dropped by GC.
"""

import os
import shutil

Import("env")  # noqa: F821 — provided by PlatformIO

LIBDEPS = env.subst("$PROJECT_LIBDEPS_DIR")  # noqa: F821
PIOENV  = env.subst("$PIOENV")               # noqa: F821
LGFX    = os.path.join(LIBDEPS, PIOENV, "LovyanGFX", "src")

V1_LVGL_STUB = """\
#pragma once
/* PATCHED by strip_lovyangfx_lvgl.py — forwards to real LVGL. */

#if defined(__has_include)
#  if __has_include(<lvgl.h>)
#    include <lvgl.h>
#    ifndef M5GFX_USING_REAL_LVGL
#      define M5GFX_USING_REAL_LVGL 1
#    endif
#  endif
#endif

/* LovyanGFX uses these aligned glyph format enum values internally but
   LVGL 9.4 does not export them. Provide them as macros so LovyanGFX
   compiles cleanly. The wrapper code that uses them is GC'd at link. */
#ifndef LV_FONT_GLYPH_FORMAT_A1_ALIGNED
#  define LV_FONT_GLYPH_FORMAT_A1_ALIGNED 0x11
#endif
#ifndef LV_FONT_GLYPH_FORMAT_A2_ALIGNED
#  define LV_FONT_GLYPH_FORMAT_A2_ALIGNED 0x12
#endif
#ifndef LV_FONT_GLYPH_FORMAT_A4_ALIGNED
#  define LV_FONT_GLYPH_FORMAT_A4_ALIGNED 0x14
#endif
#ifndef LV_FONT_GLYPH_FORMAT_A8_ALIGNED
#  define LV_FONT_GLYPH_FORMAT_A8_ALIGNED 0x18
#endif
"""

FONTS_LVGL_STUB = """\
#pragma once
/* PATCHED — was a wrapper around vendored LVGL fonts; now forwards. */
#include "../../v1/lvgl.h"
"""


def remove(path):
    if os.path.isdir(path):
        shutil.rmtree(path, ignore_errors=True)
    elif os.path.isfile(path):
        try:
            os.remove(path)
        except OSError:
            pass


def write(path, content):
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def patch_disable_block(path, start_marker, end_marker):
    if not os.path.isfile(path):
        return
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()
    if "// LVGL_STRIPPED" in text:
        return
    i = text.find(start_marker)
    if i < 0:
        return
    j = text.find(end_marker, i)
    if j < 0:
        return
    k = text.find("\n", j)
    if k < 0:
        return
    block = text[i:k + 1]
    text = text[:i] + "// LVGL_STRIPPED\n#if 0\n" + block + "#endif\n" + text[k + 1:]
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


if os.path.isdir(LGFX):
    remove(os.path.join(LGFX, "lvgl.h"))
    remove(os.path.join(LGFX, "lgfx", "v1", "lv_font"))

    write(os.path.join(LGFX, "lgfx", "v1", "lvgl.h"), V1_LVGL_STUB)

    fonts_dir = os.path.join(LGFX, "lgfx", "Fonts", "lvgl")
    if os.path.isdir(fonts_dir):
        for fn in os.listdir(fonts_dir):
            if fn.endswith(".c"):
                try:
                    os.remove(os.path.join(fonts_dir, fn))
                except OSError:
                    pass
        write(os.path.join(fonts_dir, "lvgl.h"), FONTS_LVGL_STUB)

    patch_disable_block(
        os.path.join(LGFX, "lgfx", "v1", "lgfx_fonts.cpp"),
        "    const LVGLfont lv_font_montserrat_8(",
        "    const LVGLfont lv_font_unscii_16(",
    )
    patch_disable_block(
        os.path.join(LGFX, "lgfx", "v1", "lgfx_fonts.hpp"),
        "    extern const lgfx::LVGLfont lv_font_montserrat_8;",
        "    extern const lgfx::LVGLfont lv_font_unscii_16;",
    )

    print("[strip-lvgl] LovyanGFX reconciled with real LVGL")
