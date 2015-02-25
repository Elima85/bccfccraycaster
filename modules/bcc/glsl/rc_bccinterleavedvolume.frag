#include "modules/vrn_shaderincludes.frag"

#define NORM(v) ((v) / volumeStruct_.datasetDimensions_)

const vec3 g0_off = vec3(0.0, 0.0, 0.0);
const vec3 g1_off = vec3(0.5, 0.5, 0.5);

#define TEX0(p) textureLookup3D(volumeStruct_, NORM((p) - g0_off)).r
#define TEX1(p) textureLookup3D(volumeStruct_, NORM((p) - g1_off)).g

vec4 result  = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);

uniform SAMPLER2D_TYPE     entryPoints_;
uniform SAMPLER2D_TYPE     entryPointsDepth_;
uniform TEXTURE_PARAMETERS entryParameters_;
uniform SAMPLER2D_TYPE     exitPoints_;
uniform SAMPLER2D_TYPE     exitPointsDepth_;
uniform TEXTURE_PARAMETERS exitParameters_;

uniform VOLUME_STRUCT volumeStruct_;

vec3 convert = vec3(0.5, 0.5, 1.0); //conversion between 3D coords and texture index
vec3 oneOverVoxels = vec3(1.0)/vec3(volumeStruct_.datasetDimensions_-1.0);
vec3 size_volume = vec3(2*volumeStruct_.datasetDimensions_.x-1.0, 2*volumeStruct_.datasetDimensions_.y-1.0,
	volumeStruct_.datasetDimensions_.z-1.0);

vec4 reconstructLinbox(vec3 p)
{
	vec3 P1, P2, P3, P4;
	float D1, D2, D3, D4;
	
	vec3 posOS = (p * size_volume);

	vec3 abc = vec3(posOS.x + posOS.y,
	                posOS.x + posOS.z,
	                posOS.y + posOS.z) * 0.5;

	vec3 floors = floor(abc);
	abc = abc - floors;

	P1 = vec3( floors.x + floors.y - floors.z,
	           floors.x - floors.y + floors.z,
	          -floors.x + floors.y + floors.z);

	P2 = P1 + vec3(1.0, 1.0,  1.0);
	P3 = P1 + vec3(1.0, 1.0, -1.0);
	P4 = P1 + vec3(2.0, 0.0,  0.0);

	vec4 sorting = vec4(1.0, 0.0, 0.0, 0.0);

	sorting.y = max(abc.x, max(abc.y, abc.z));
	sorting.z = min(abc.x, min(abc.y, abc.z));
	sorting.w = (abc.x + abc.y + abc.z) - sorting.y - sorting.z;

	P3 += (int(sorting.y == abc.y) * vec3( 0.0, -2.0, 2.0) +
	       int(sorting.y == abc.z) * vec3(-2.0,  0.0, 2.0));

	P4 += (int(sorting.z == abc.x) * vec3(-2.0,  0.0, 2.0) +
	       int(sorting.z == abc.y) * vec3(-2.0,  2.0, 0.0));
	
	D1 = textureLookup3D(volumeStruct_, (floor(P1*convert)) * oneOverVoxels).a;
	D2 = textureLookup3D(volumeStruct_, (floor(P2*convert)) * oneOverVoxels).a;
	D3 = textureLookup3D(volumeStruct_, (floor(P3*convert)) * oneOverVoxels).a;
	D4 = textureLookup3D(volumeStruct_, (floor(P4*convert)) * oneOverVoxels).a;
	
	/*D1 = texture(volumeStruct_.volume_, (P1*convert) * oneOverVoxels).a;
	D2 = texture(volumeStruct_.volume_, (P2*convert) * oneOverVoxels).a;
	D3 = texture(volumeStruct_.volume_, (P3*convert) * oneOverVoxels).a;
	D4 = texture(volumeStruct_.volume_, (P4*convert) * oneOverVoxels).a;*/
	
	vec4 values = vec4(D1, D3-D1, D2-D4, D4-D3);
	return vec4(0.0, 0.0, 0.0, dot(sorting, values));
	
}

vec4 reconstructNearest(in vec3 p)
{
	//vec3 pw = p * volumeStruct_.datasetDimensions_ - 0.5;
	vec3 pw = p * size_volume;
	
	vec4 value = textureLookup3D(volumeStruct_, (floor(pw*convert)) * oneOverVoxels);

	return value;
}

void rayTraversal(in vec3 first, in vec3 last)
{
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, volumeStruct_.datasetDimensions_, rayDirection, tIncr, tEnd);
    tIncr /= 1.259921049894873;
	
    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = RC_APPLY_RECONSTRUCTION(samplePos);
		
        vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);

        color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeStruct_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));
		//color.rgb = samplePos;

        if (color.a > 0.0) {
            RC_BEGIN_COMPOSITING
            result = RC_APPLY_COMPOSITING(result, color, samplePos, voxel.xyz, t, tDepth)
            result1 = RC_APPLY_COMPOSITING_1(result1, color, samplePos, voxel.xyz, t, tDepth)
            result2 = RC_APPLY_COMPOSITING_2(result2, color, samplePos, voxel.xyz, t, tDepth)
            RC_END_COMPOSITING
        }
    } RC_END_LOOP(result);
}

void main()
{
    vec3 frontPos = textureLookup2D(entryPoints_, entryParameters_, gl_FragCoord.xy).rgb;
    vec3 backPos = textureLookup2D(exitPoints_, exitParameters_, gl_FragCoord.xy).rgb;
	
    if (frontPos == backPos)
        discard;
	else
        rayTraversal(frontPos, backPos);

	#ifdef OP0
		FragData0 = result;
	#endif
	#ifdef OP1
		FragData1 = result1;
	#endif
	#ifdef OP2
		FragData2 = result2;
	#endif
}

