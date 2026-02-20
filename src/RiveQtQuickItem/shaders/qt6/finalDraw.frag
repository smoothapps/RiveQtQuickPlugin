// SPDX-FileCopyrightText: 2023 Jeremias Bosch <jeremias.bosch@basyskom.com>
// SPDX-FileCopyrightText: 2023 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 texCoord;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    int flipped;
    float left;
    float right;
    float top;
    float bottom;
    int useTextureNumber;
    int ditherMode;
};

layout(binding = 1) uniform sampler2D u_textureA;
layout(binding = 2) uniform sampler2D u_textureB;
layout(binding = 3) uniform sampler2D u_texturePP;

vec4 drawTexture(sampler2D s_texture, vec2 texCoord) {
    if (texCoord.x >= left && texCoord.x <= right &&
        texCoord.y >= top && texCoord.y <= bottom) {
        return texture(s_texture, texCoord);
    } else {
        return vec4(0.0, 0.0, 0.0, 0.0);  // Return a transparent color for pixels outside the viewport
    }
}

// Interleaved gradient noise for dithering
float interleavedGradientNoise(vec2 fragCoord)
{
    float v1 = fract(0.06711056 * fragCoord.x + 0.00583715 * fragCoord.y);
    float v2 = fract(52.9829189 * v1);
    return v2;
}

vec4 applyDither(vec4 color, vec2 fragCoord, int ditherMode)
{
    if (ditherMode == 0) { // DitherNone
        return color;
    }
    
    // DitherInterleavedGradientNoise
    float noise = interleavedGradientNoise(fragCoord);
    // Scale noise to [-0.5/255, 0.5/255] range for 8-bit color channels
    float dither = (noise - 0.5) / 255.0;
    
    return vec4(color.rgb + dither, color.a);
}

void main()
{
    if (useTextureNumber == 0) {
        vec4 finalColor = drawTexture(u_textureA, texCoord);
        fragColor = applyDither(finalColor * qt_Opacity, gl_FragCoord.xy, ditherMode);
    }
    if (useTextureNumber == 1) {
        vec4 finalColor = drawTexture(u_textureB, texCoord);
        fragColor = applyDither(finalColor * qt_Opacity, gl_FragCoord.xy, ditherMode);
    }
    if (useTextureNumber == 2) {
        vec4 finalColor = drawTexture(u_texturePP, texCoord);
        fragColor = applyDither(finalColor * qt_Opacity, gl_FragCoord.xy, ditherMode);
    }
}
