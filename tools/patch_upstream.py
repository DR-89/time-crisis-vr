"""Small, repeatable Quest hooks; keep the shared engine and generated game intact."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1] / 'upstream'

def edit(name, changes, marker='TCVR_PATCH'):
    path = ROOT / name
    text = path.read_text(encoding='utf-8')
    if marker in text:
        return
    for old, new in changes:
        if old not in text:
            raise RuntimeError(f'Upstream changed: {name}: {old[:70]}')
        text = text.replace(old, new)
    path.write_text(f'/* {marker}: guarded Quest integration. */\n' + text, encoding='utf-8')

def main():
    edit('timecris/gen/tc_lifted_tab.c', [
        ('  rr_trap(at, t, "jump to an address that is no known instruction");', '''#ifdef TCVR
  extern void (*tc_crate_entry(uint32_t))(uint32_t);
  void (*extra)(uint32_t)=tc_crate_entry(t);
  if(extra){extra(t);return;}
#endif
  rr_trap(at, t, "jump to an address that is no known instruction");'''),
        ('  return 0;\n}', '''#ifdef TCVR
  extern void (*tc_crate_entry(uint32_t))(uint32_t);
  return tc_crate_entry(ep);
#else
  return 0;
#endif
}'''),
    ], marker='TCVR_CRATE_COROUTINE')
    edit('engine/geo_hw.h', [('    int      direct;', '    float    vr_focal, vr_cx, vr_cy; /* original camera projection, for stereo reconstruction */\n    int      direct;')])
    edit('engine/geo_hw.c', [('    q.color = (uint32_t)color;', '    q.vr_focal = ldexpf((float)mant, -shift);\n    q.vr_cx = (float)cx; q.vr_cy = (float)cy;\n    q.color = (uint32_t)color;')])
    edit('engine/ss22_gl.c', [
        ('#include "eng_gl.h"', '#include "eng_gl.h"\n#ifdef TCVR\n#include "quest_scene.h"\n#endif'),
        ('    qn = 0; qorder = 0;', '    qn = 0; qorder = 0;\n#ifdef TCVR\n    qvr_scene_begin();\n#endif'),
        ('    ss22_qhist_report();', '    #ifdef TCVR\n    for (int i = 0; i < qn; ++i) qvr_scene_quad(&qbuf[i]);\n    #endif\n    ss22_qhist_report();'),
    ])
    # Expand position arrays only in the Quest build. Original texture/fog batching remains shared.
    edit('engine/quad_gl.c', [
        ('#include "eng_gl.h"', '#include "eng_gl.h"\n#ifdef TCVR\n#include "quest_scene.h"\n#define QPOS 4\n#else\n#define QPOS 2\n#endif'),
        ('    p_active_texture = (void (APIENTRY *)(GLenum))SDL_GL_GetProcAddress("glActiveTexture");', '#ifdef TCVR\n    p_active_texture = glActiveTexture;\n#else\n    p_active_texture = (void (APIENTRY *)(GLenum))SDL_GL_GetProcAddress("glActiveTexture");\n#endif'),
        ('qa_xy[32 * 2]', 'qa_xy[32 * QPOS]'),
        ('bb_xy[BB_MAXV * 2]', 'bb_xy[BB_MAXV * QPOS]'),
        ('bb_xy[bb_n*2+0] = qa_xy[s*2+0]; bb_xy[bb_n*2+1] = qa_xy[s*2+1];', 'memcpy(&bb_xy[bb_n*QPOS], &qa_xy[s*QPOS], QPOS * sizeof(float));'),
        ('glVertexPointer(2, GL_FLOAT, 0, bb_xy);', 'glVertexPointer(QPOS, GL_FLOAT, 0, bb_xy);'),
        ('qa_xy[i*2+0] = q->rv[i].sx16 / 16.0f; qa_xy[i*2+1] = q->rv[i].sy16 / 16.0f;', '#ifdef TCVR\n            qvr_vertex(q, q->rv[i].sx16 / 16.0f, q->rv[i].sy16 / 16.0f, (float)q->rv[i].z, &qa_xy[i*QPOS]);\n#else\n            qa_xy[i*2+0] = q->rv[i].sx16 / 16.0f; qa_xy[i*2+1] = q->rv[i].sy16 / 16.0f;\n#endif'),
        ('qa_xy[i*2+0] = cv[i].x; qa_xy[i*2+1] = cv[i].y;', '#ifdef TCVR\n            qvr_vertex(q, cv[i].x, cv[i].y, cv[i].w > 0 ? 1.0f/cv[i].w : 1.0f, &qa_xy[i*QPOS]);\n#else\n            qa_xy[i*2+0] = cv[i].x; qa_xy[i*2+1] = cv[i].y;\n#endif'),
        ('    int ncv = clip_to_screen(sv, nsv, cv);', '    int ncv;\n#ifdef TCVR\n    if (!q->direct && q->vr_focal > 0) { ncv = nsv; memcpy(cv, sv, sizeof(*cv)*nsv); }\n    else\n#endif\n    ncv = clip_to_screen(sv, nsv, cv);'),
    ])
    edit('engine/ss22_gl.c', [
        ('    uint64_t h = 1469598103934665603ULL;', '''    uint64_t h = 1469598103934665603ULL;
#ifdef TCVR
    /* RAM is unchanged between stereo eyes and repeated XR poses. Hash once
     * per prepared game frame, while retaining the original change detection. */
    static int hash_frame = -1; static uint64_t frame_hash;
    if (hash_frame == frames) h = frame_hash;
    else {
#endif'''),
        ('#undef MIX', '''#undef MIX
#ifdef TCVR
        frame_hash = h; hash_frame = frames;
    }
#endif'''),
    ], marker='TCVR_TEXT_FRAME_CACHE')
    edit('engine/ss22_gl.c', [
        ('#include "quest_scene.h"', '#include "quest_scene.h"\n#include "quest_gl.h"'),
        ('    glViewport(0, 0, vw, vh);', '''#ifdef TCVR
    qgl_scene_begin(g_fog_valid && g_fog.have_gamma ? g_fog.gamma : NULL, vw, vh);
#endif
    glViewport(0, 0, vw, vh);'''),
    ], marker='TCVR_SCENE_TARGET')
    edit('engine/tex_bake.c', [
        ('#include "eng_gl.h"', '#include "eng_gl.h"\n#ifdef TCVR\n#include "quest_gl.h"\n#endif'),
        ('    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_DIM, ATLAS_DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);', '''#ifdef TCVR
    qgl_atlas_texture(apages[i].tex, i, ATLAS_DIM, atlas_maxp);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_DIM, ATLAS_DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);'''),
    ], marker='TCVR_ATLAS_ARRAY')
    edit('engine/ss22_gl.c', [
        ('static uint8_t *spr_buf, *txt_buf;', '''static uint8_t *spr_buf, *txt_buf;
#ifdef TCVR
/* Each prepared game frame is displayed by both eyes and sometimes another XR
 * pose. Keep its sprite images immutable until the next game frame: this avoids
 * rasterizing and overwriting one shared texture several times per sprite. */
static struct { GLuint tex; int w, h, frame; } spr_cache[1024];
static size_t spr_cache_bytes;
static int spr_cache_frame = -1;
static void sprite_cache_begin(void)
{
    if (spr_cache_frame == frames) return;
    spr_cache_frame = frames;
    for (int i = 0; i < 1024; i++) {
        if (spr_cache[i].tex && (i >= ni || spr_cache[i].w != items[i].w || spr_cache[i].h != items[i].h)) {
            glDeleteTextures(1, &spr_cache[i].tex);
            spr_cache_bytes -= (size_t)spr_cache[i].w * spr_cache[i].h * 4;
            memset(&spr_cache[i], 0, sizeof spr_cache[i]);
        }
    }
    if (prio_any) { memset(prio_mask, 0, sizeof prio_mask); prio_any = false; }
}
#endif'''),
        ('    sprite_render_item(&sst, &g_fog, it, spr_buf, 1);', '''#ifdef TCVR
    int slot = (int)(it - items);
    bool cached = spr_cache[slot].tex && spr_cache[slot].frame == frames;
    if (!cached) {
#endif
    sprite_render_item(&sst, &g_fog, it, spr_buf, 1);'''),
        ('    if (!spr_tex) tex_alloc(&spr_tex); else glBindTexture(GL_TEXTURE_2D, spr_tex);\n    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, it->w, it->h, GL_RGBA, GL_UNSIGNED_BYTE, spr_buf);\n    const float su = (float)it->w / SPR_W, sv = (float)it->h / SPR_H;', '''#ifdef TCVR
        size_t bytes = (size_t)it->w * it->h * 4;
        if (!spr_cache[slot].tex && spr_cache_bytes + bytes <= 32u * 1024u * 1024u) {
            glGenTextures(1, &spr_cache[slot].tex);
            glBindTexture(GL_TEXTURE_2D, spr_cache[slot].tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, it->w, it->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            spr_cache[slot].w = it->w; spr_cache[slot].h = it->h; spr_cache_bytes += bytes;
        }
        if (spr_cache[slot].tex) {
            glBindTexture(GL_TEXTURE_2D, spr_cache[slot].tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, it->w, it->h, GL_RGBA, GL_UNSIGNED_BYTE, spr_buf);
            spr_cache[slot].frame = frames;
        }
    }
    const bool stored = spr_cache[slot].tex != 0;
    if (stored) glBindTexture(GL_TEXTURE_2D, spr_cache[slot].tex);
    else {
#endif
    if (!spr_tex) tex_alloc(&spr_tex); else glBindTexture(GL_TEXTURE_2D, spr_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, it->w, it->h, GL_RGBA, GL_UNSIGNED_BYTE, spr_buf);
#ifdef TCVR
    }
    const float su = stored ? 1.0f : (float)it->w / SPR_W, sv = stored ? 1.0f : (float)it->h / SPR_H;
#else
    const float su = (float)it->w / SPR_W, sv = (float)it->h / SPR_H;
#endif'''),
        ('    if (prio_any) { memset(prio_mask, 0, sizeof prio_mask); prio_any = false; }\n\n    /* the scene', '''#ifdef TCVR
    sprite_cache_begin();
#else
    if (prio_any) { memset(prio_mask, 0, sizeof prio_mask); prio_any = false; }
#endif

    /* the scene'''),
    ], marker='TCVR_SPRITE_FRAME_CACHE')
    edit('engine/sprite_hw.c', [
        ('typedef struct { uint8_t *rgba; uint8_t *prio; int ox, oy, w, h; } tile_dst;', '''typedef struct { uint8_t *rgba; uint8_t *prio; int ox, oy, w, h; } tile_dst;
#ifdef TCVR
#include "quest_sprite_raster.h"
#endif'''),
        ('    int y0 = sizey > 0 ? sy : sy + sizey + 1;', '''    int y0 = sizey > 0 ? sy : sy + sizey + 1;
#ifdef TCVR
    if (c->compose_alpha) { qvr_sprite_tile(dst, pal, pal_base, tilep, flipx, flipy, x0, y0, ssw, ssh, c); return; }
#endif'''),
    ], marker='TCVR_SPRITE_PALETTE')
    edit('engine/text_hw.c', [
        ('void text_render(const text_state *st, const fog_state *fog,', '''#ifdef TCVR
#include "quest_text_raster.h"
#endif
void text_render(const text_state *st, const fog_state *fog,'''),
        ('    if (!st || !st->valid) return;', '''    if (!st || !st->valid) return;
#ifdef TCVR
    if (!gate_mode) { qvr_text_render(st, fog, rgba); return; }
#endif'''),
    ], marker='TCVR_TEXT_PALETTE')
    edit('engine/ss22_gl.c', [
        ('i >= ni || spr_cache[i].w != items[i].w || spr_cache[i].h != items[i].h', 'i >= ni || spr_cache[i].w < items[i].w || spr_cache[i].h < items[i].h'),
        ('        size_t bytes = (size_t)it->w * it->h * 4;', '''        /* Rounded capacity keeps zooming sprites from reallocating every frame. */
        int cw = (it->w + 15) & ~15, ch = (it->h + 15) & ~15;
        size_t bytes = (size_t)cw * ch * 4;'''),
        ('glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, it->w, it->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);', 'glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cw, ch, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);'),
        ('spr_cache[slot].w = it->w; spr_cache[slot].h = it->h;', 'spr_cache[slot].w = cw; spr_cache[slot].h = ch;'),
        ('const float su = stored ? 1.0f : (float)it->w / SPR_W, sv = stored ? 1.0f : (float)it->h / SPR_H;', 'const float su = (float)it->w / (stored ? spr_cache[slot].w : SPR_W), sv = (float)it->h / (stored ? spr_cache[slot].h : SPR_H);'),
    ], marker='TCVR_SPRITE_CAPACITY')
    edit('engine/ss22_gl.c', [
        ('static size_t spr_cache_bytes;', '''#ifndef QVR_SPRITE_CACHE_BYTES
#define QVR_SPRITE_CACHE_BYTES (32u * 1024u * 1024u)
#endif
static size_t spr_cache_bytes;'''),
        ('spr_cache_bytes + bytes <= 32u * 1024u * 1024u', 'spr_cache_bytes + bytes <= QVR_SPRITE_CACHE_BYTES'),
    ], marker='TCVR_SPRITE_BUDGET')

if __name__ == '__main__':
    main()
