#ifdef PYRA_GLES2
attribute vec3 coord3d;
#else
in vec3 coord3d;
#endif
#ifdef PYRA_GLES2
attribute vec4 v_color;
#else
in vec4 v_color;
#endif
#ifdef PYRA_GLES2
attribute float v_light;
#else
in float v_light;
#endif
#ifdef PYRA_GLES2
attribute vec2 v_uv;
#else
in vec2 v_uv;
#endif
uniform mat4 mvp;
#ifdef PYRA_GLES2
varying vec4 f_color;
#else
out vec4 f_color;
#endif
#ifdef PYRA_GLES2
varying float f_light;
#else
out float f_light;
#endif
#ifdef PYRA_GLES2
varying vec2 f_uv;
#else
out vec2 f_uv;
#endif
#ifdef PYRA_GLES2
varying vec3 f_coord;
#else
out vec3 f_coord;
#endif

void main(void)
{
    vec4 pos = mvp * vec4(coord3d, 1.0);
    pos.w = 1.0/(1.0-coord3d.z);
    pos.xyz *= pos.w;
    gl_Position = pos;
    f_color = v_color.bgra;
    f_uv = v_uv;
    f_coord = coord3d;
    f_light = v_light;
}
