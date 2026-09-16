#ifdef PYRA_GLES2
#define fragColor gl_FragColor
#endif
#ifndef PYRA_GLES2
#define texture2D texture
#endif
uniform sampler2D tex;
uniform vec2 iResolution;
uniform float param1;
uniform float param2;
uniform float param3;
#ifdef PYRA_GLES2
varying vec4 f_color;
#else
in vec4 f_color;
#endif
#ifdef PYRA_GLES2
varying vec2 f_uv;
#else
in vec2 f_uv;
#endif
#ifdef PYRA_GLES2
varying vec3 f_coord;
#else
in vec3 f_coord;
#endif
#ifndef PYRA_GLES2
out vec4 fragColor;
#endif

void main(void)
{
    vec4 sampled_color = texture2D(tex, f_uv);
    vec4 vertex_color = f_color;
    vec4 blend = vec4(param2, param2, param2, param1);

    fragColor = sampled_color * vertex_color * blend;
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/param3));
    fragColor.rgba = clamp(fragColor.rgba, 0.0, 1.0);
    //fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
