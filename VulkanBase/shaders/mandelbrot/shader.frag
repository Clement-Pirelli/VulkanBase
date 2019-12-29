#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform UniformBufferObject
{
    vec2 resolution;
} ubo;

#define iResolution ubo.resolution

const int MAX_ITER = 500;

vec3 BASE_COLOR = vec3(205.0,100.0, 128.0) / 255.0;


struct complexNumber
{
    float real;
    float imaginary;
};

complexNumber multiplyComplex(complexNumber a, complexNumber b)
{
	//(x + yi) * (u + vi) = (xu - yv) + (xv + yu)i
    
    return complexNumber
        (
        	a.real*b.real - a.imaginary*b.imaginary,
            a.real*b.imaginary + b.real*a.imaginary
        );
}

complexNumber toComplex(vec2 a)
{
	return complexNumber(a.x, a.y);
}

complexNumber addComplex(complexNumber a, complexNumber b)
{
	return complexNumber(a.real + b.real, a.imaginary + b.imaginary);
}

float modulusComplex(complexNumber a)
{
	return sqrt(a.real*a.real + a.imaginary*a.imaginary);
}

//original code from : http://www.chilliant.com/rgb2hsv.html
const float Epsilon = 1e-10;
 
vec3 RGBtoHCV(in vec3 RGB)
{
    // Based on work by Sam Hocevar and Emil Persson
    vec4 P = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0/3.0) : vec4(RGB.gb, 0.0, -1.0/3.0);
    vec4 Q = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
    float C = Q.x - min(Q.w, Q.y);
    float H = abs((Q.w - Q.y) / (6. * C + Epsilon) + Q.z);
	return vec3(H, C, Q.x);
}

//original code from : http://www.chilliant.com/rgb2hsv.html
vec3 RGBtoHSV(in vec3 RGB)
{
    vec3 HCV = RGBtoHCV(RGB);
    float S = HCV.y / (HCV.z + Epsilon);
    return vec3(HCV.x, S, HCV.z);
}

//original code from : http://www.chilliant.com/rgb2hsv.html
vec3 HUEtoRGB(in float H)
{
	float R = abs(H * 6. - 3.) - 1.;
    float G = 2. - abs(H * 6. - 2.);
    float B = 2. - abs(H * 6. - 4.);
    return clamp(vec3(R,G,B), .0, 1.);
}

//original code from : http://www.chilliant.com/rgb2hsv.html
vec3 HSVtoRGB(in vec3 HSV)
{
    vec3 RGB = HUEtoRGB(HSV.x);
    return ((RGB - 1.) * HSV.y + 1.) * HSV.z;
}

//code from Unity's Mathf.PingPong
float pingpong(float t, float l)
{
    t = mod(t, l * 2.);
    return l - abs(t - l);
}

const int aa = 2;
void main()
{
    
    vec3 total = vec3(.0);
    for(int i = 0; i < aa; i++)
    for(int j = 0; j < aa; j++)
    {
    vec2 uv = (gl_FragCoord.xy+vec2(float(i),float(j))/float(aa)-.5)/iResolution.xy - vec2(.5);
    	uv.x *= iResolution.x/iResolution.y;
    
    uv *= 1.0/ pow(2.0, pingpong(3.24+1.0, 16.0));
	complexNumber C = toComplex(uv);
	C.real += 0.281717921930775;
    C.imaginary += 0.5771052841488505;
    
    complexNumber Z = toComplex(vec2(.0));
    int itn = 0;
    
    for(int i = 0; i < MAX_ITER; i++)
    {
        Z =  addComplex(multiplyComplex(Z,Z), C);
        
        if(modulusComplex(Z) >= 2.0) 
        {
        	itn = i;
            break;
        }
    }
    
    float itRatio = float(itn) / float(MAX_ITER);
    
    vec3 col = BASE_COLOR;
    col = RGBtoHSV(col);
    col.x *= itRatio;
    col = HSVtoRGB(col);
    
        total += col;
    
    }
    total /= float(aa*aa);
  
    
    // Output to screen
    outColor = vec4(total,1.0);
}