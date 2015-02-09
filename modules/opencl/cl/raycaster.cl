/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "mod_samplers.cl" 
#include "mod_gradients.cl"

__constant float SAMPLING_BASE_INTERVAL_RCP = 200.0;

/**
 * Makes a simple raycast through the volume for entry to exit point with minimal diffuse shading.
 */
float4 simpleRaycast(__global read_only image3d_t volumeTex, __global read_only image2d_t tfData, const float4 entryPoint, const float4 exitPoint, float* depth, const float stepSize) {

    // result color
    float4 result = (float4)(0.0, 0.0, 0.0, 0.0);

    float t = 0.0; //the current position on the ray with entryPoint as the origin
    float4 direction = exitPoint - entryPoint; //the direction of the ray
    direction.w = 0.0;
    float tend = fast_length(direction); //the length of the ray

    direction = fast_normalize(direction);

    while(t <= tend) {

        //calculate sample position and get corresponding voxel
        float4 sample = entryPoint + t * direction;
        float volumeSample = getVoxel(volumeTex, smpNorm, as_float4(sample));     
        float4 color = read_imagef(tfData, smpNone, (float2)(toDataSpace1d(1.f/(float)get_image_width(tfData), volumeSample),0.5f));

        // apply opacity correction to accomodate for variable sampling intervals
        color.w = 1.0 - pow(1.0 - color.w, stepSize * SAMPLING_BASE_INTERVAL_RCP);

        // Add a little shading.  calcGradient is declared in mod_gradients.cl
        float4 norm = normalize(calcGradient(sample, volumeTex));
        color *= fabs(dot(norm, direction));

        //calculate ray integral
        result.xyz = result.xyz + (1.0 - result.w) * color.w * color.xyz;
        result.w = result.w + (1.0 - result.w) * color.w;

        // early ray termination
        if(result.w > 0.95)
            break;

        //raise position on ray
        t += stepSize;
    }

    // TODO: calculate correct depth value
    if(t >= 0.0)
        *depth = t / tend;
    else
        *depth = 1.0;

    return result;
}

//main for raycasting. This function is called for every pixel in view.
// TODO: Depth values are currently not read or written as OpenCL does not support OpenGL GL_DEPTH_COMPONENT image formats.
__kernel void raycast(__global read_only image3d_t volumeTex,
                      __global read_only image2d_t tfData,
                      __global read_only image2d_t entryTexCol,
                      __global read_only image2d_t exitTexCol,
                      __global write_only image2d_t outCol,
                      //__global read_only image2d_t entryTexDepth,
                      //__global read_only image2d_t exitTexDepth,
                      //__global write_only image2d_t outDepth,
                      float stepSize
                      )
{
    //output image pixel coordinates
    int2 target = (int2)(get_global_id(0), get_global_id(1));
    // Need to add 0.5 in order to get the correct coordinate. We could also use the integer coordinate directly...
    float2 targetNorm = (convert_float2(target) + (float2)(0.5)) / convert_float2((int2)(get_global_size(0), get_global_size(1)));

    float4 color;
    float depth = 1.0;

    float4 entry = read_imagef(entryTexCol, smpNorm, targetNorm);
    float4 exit  = read_imagef(exitTexCol,  smpNorm, targetNorm);

    if( entry.x != exit.x || entry.y != exit.y || entry.z != exit.z )
        color = simpleRaycast(volumeTex, tfData, entry, exit, &depth, stepSize);
    else
        color = (float4)(0.0);

    write_imagef(outCol, target, color);
    //write_imagef(outDepth, target, (float4)(depth));
}


