#include "engine/shader/version.glsl"
#include "engine/shader/depth.glsl"

out vec4 FragColor;

uniform float u_near;
uniform float u_far;

in vec4 position_vs_;

void main() { 
// Method 1 same as Method 2
  float depth_vs = GetLinearDepthFromSS(gl_FragCoord.z, u_near, u_far);
  float normalized_depth_vs = NormalizeLinearDepth(depth_vs, u_near, u_far);
// Method 2 same as Method 1
//  float depth_vs = -position_vs_.z;
//  float normalized_depth_vs = NormalizeLinearDepth(depth_vs, u_near, u_far);
  FragColor = vec4(vec3(normalized_depth_vs), 1.0);
}