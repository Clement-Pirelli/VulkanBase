#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution

#define CELL_SIZE .15

struct voronoiReturn
{
	float distToEdge;
    float colorRand;
};
    
vec2 pseudorandPos(vec2 randseed)
{
    return abs(cos(fract(randseed * dot(sin(randseed * 242.589), vec2(42.4391588, 3.91711988))) * 294419.4857));
}

float pseudorandScalar(float randseed)
{
	return fract(dot(vec3(sin(randseed*194.0154)), vec3(1043.94219, 4920.22948, 744.2918)));
}

float pseudorandScalar(vec2 randseed)
{
	return fract(dot(vec2(sin(randseed*194.0154)), vec2(1043.94219, 744.2918)));
}

//based on : https://www.ronja-tutorials.com/2018/09/29/voronoi-noise.html
voronoiReturn getCell(vec2 pos)
{
    vec2 baseCell = floor(pos);
    float minDistToCell = 10000.0;
    vec2 closestCell = vec2(.0);
    vec2 toClosestCell = vec2(.0);
    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
        	vec2 cell = baseCell + vec2(i,j);
        	vec2 cellPos = cell + pseudorandPos(cell);
        	vec2 toCell = cellPos - pos;
        	float distToCell = length(toCell);
            if(distToCell < minDistToCell){
                minDistToCell = distToCell;
                closestCell = cell;
                toClosestCell = toCell;
            }
        }
    }
    
    
    
    float minEdgeDistance = 10.;
    for(int x2=-1; x2<=1; x2++){
        for(int y2=-1; y2<=1; y2++){
            vec2 cell = baseCell + vec2(x2, y2);
            vec2 cellPosition = cell + pseudorandPos(cell);
            vec2 toCell = cellPosition - pos;

            vec2 diffToClosestCell = abs(closestCell - cell);
            bool isClosestCell = diffToClosestCell.x + diffToClosestCell.y < 0.1;
            if(!isClosestCell){
                vec2 toCenter = (toClosestCell + toCell) * 0.5;
                vec2 cellDifference = normalize(toCell - toClosestCell);
                float edgeDistance = dot(toCenter, cellDifference);
                minEdgeDistance = min(minEdgeDistance, edgeDistance);
            }
        }
    }
    
    
	return voronoiReturn(minEdgeDistance,pseudorandScalar(closestCell));
}

vec3 getColor(float v)
{
	return vec3(.1, .7, .1) + vec3(0.2, 0.3, 0.2) * cos(2.*3.141*(vec3(1.,2.0,1.0)*v+vec3(.01,.02,.01)));
}


void main()
{
    vec2 uv = gl_FragCoord.xy/iResolution.xy - .5;
	uv.x *= iResolution.x/iResolution.y;
    
    
    uv /= CELL_SIZE;
    
    
    voronoiReturn v = getCell(uv);
    
    
	//based on : https://www.ronja-tutorials.com/2018/09/29/voronoi-noise.html
    float valueChange = length(fwidth(uv)) * 0.5;
	float isBorder = 1.0 - smoothstep(0.08 - valueChange, 0.08 + valueChange, v.distToEdge);
	vec3 col = mix(getColor(v.colorRand), vec3(.0), isBorder);
    
    
    
    col = mix(col, vec3(.0), .1*(isBorder+1.0));
    

    outColor = vec4( pow(col, vec3(1./2.2))  ,1.0);
    //
}