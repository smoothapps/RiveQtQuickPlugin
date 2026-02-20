// SPDX-FileCopyrightText: 2023 Jeremias Bosch <jeremias.bosch@basyskom.com>
// SPDX-FileCopyrightText: 2023 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 originalVertex;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;                     //0
    float qt_Opacity;                   //64
    float gradientRadius;               //68
    int useGradient;                    //72
    int useTexture;                     //76
    vec2 gradientFocalPoint;            //80
    vec2 gradientCenter;                //88
    vec2 startPoint;                    //96
    vec2 endPoint;                      //104
    int numberOfStops;                  //112
    int gradientType;                   //116
    vec4 color;                         //128
    vec4 stopColors[20];                //144
    vec2 gradientPositions[20];         //464
    mat4 tranformMatrix;                //784
    int ditherMode;                     //848
};
layout(binding = 1) uniform sampler2D image;

vec4 getGradientColor( float gradientCoord) {
    vec4 gradientColor = stopColors[0];

    if (gradientCoord >= gradientPositions[numberOfStops - 1].x) {
        return stopColors[numberOfStops - 1];
    }

    for (int i = 1; i < numberOfStops; ++i) {
        if (gradientCoord <= gradientPositions[i].x) {
            gradientColor = mix(stopColors[i - 1], stopColors[i], smoothstep(gradientPositions[i - 1].x, gradientPositions[i].x, gradientCoord));
            break;
        }
    }
    return gradientColor;
}

// Interleaved gradient noise for dithering
// Based on: https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
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
    if (useTexture == 1) {
        fragColor = texture(image, texCoord);
        //fragColor = vec4(1,0,0,1);
    } else {
        if (useGradient == 1) {
            float gradientCoord;
            if (gradientType == 0) { // Linear gradient
                vec2 dir = originalVertex - startPoint;
                vec2 gradientDir = endPoint - startPoint;
                gradientCoord = dot(dir, gradientDir) / dot(gradientDir, gradientDir);
            } else { // Radial gradient
                vec2 dir = originalVertex - gradientCenter;
                gradientCoord = length(dir) / gradientRadius;
            }
            gradientCoord = clamp(gradientCoord, 0.0, 1.0);
            fragColor = getGradientColor(gradientCoord);
        } else {
            fragColor = color;
        }
    }
    fragColor = fragColor * qt_Opacity;
    fragColor = applyDither(fragColor, gl_FragCoord.xy, ditherMode);
}
