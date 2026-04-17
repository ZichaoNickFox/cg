#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/geometry.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/transform.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D canvas;

uniform vec2 resolution;
uniform Camera camera;
uniform Sphere spheres[10];
uniform int frame_num;

struct PathHit {
  Sphere sphere;
  RaySphereResult hit;
};

PathHit TraceSpheres(Ray ray) {
  PathHit res;
  float t_min = 1e20;
  for (int i = 0; i < 10; ++i) {
    Sphere sphere = spheres[i];
    RaySphereResult hit = RaySphere(sphere, ray, 1000.0);
    if (hit.hitted && hit.distance <= camera.far && hit.distance < t_min) {
      t_min = hit.distance;
      res.sphere = sphere;
      res.hit = hit;
    }
  }
  return res;
}

vec3 RandomUnitVector() {
  vec3 v = vec3(Random() * 2.0 - 1.0, Random() * 2.0 - 1.0, Random() * 2.0 - 1.0);
  return normalize(v);
}

vec3 SampleHemisphere(vec3 normal) {
  vec3 dir = RandomUnitVector();
  return dot(dir, normal) < 0.0 ? -dir : dir;
}

vec4 TracePath(vec3 ray_origin, vec3 ray_dir) {
  vec3 throughput = vec3(1.0);
  vec3 radiance = vec3(0.0);

  for (int bounce = 0; bounce < 4; ++bounce) {
    PathHit path_hit = TraceSpheres(Ray(ray_origin, ray_dir));
    if (!path_hit.hit.hitted) {
      radiance += throughput * vec3(0.03, 0.05, 0.08);
      break;
    }

    vec3 normal = normalize(path_hit.hit.pos - path_hit.sphere.center_pos);
    vec3 albedo = path_hit.sphere.color.rgb;
    if (path_hit.sphere.id == 1) {
      radiance += throughput * albedo * 6.0;
      break;
    }

    throughput *= albedo;
    if (path_hit.sphere.id == 8 || path_hit.sphere.id == 10) {
      ray_dir = reflect(ray_dir, normal);
    } else if (path_hit.sphere.id == 9) {
      float eta = dot(ray_dir, normal) < 0.0 ? (1.0 / 1.52) : 1.52;
      vec3 oriented_normal = dot(ray_dir, normal) < 0.0 ? normal : -normal;
      ray_dir = refract(ray_dir, oriented_normal, eta);
      if (length(ray_dir) < 1e-5) {
        ray_dir = reflect(ray_dir, oriented_normal);
      }
    } else {
      ray_dir = SampleHemisphere(normal);
    }
    ray_origin = path_hit.hit.pos + normal * 0.001;
  }

  return vec4(radiance, 1.0);
}

void main() {
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
  if (pixel.x >= int(resolution.x) || pixel.y >= int(resolution.y)) {
    return;
  }

  InitRNG(gl_GlobalInvocationID.xy, frame_num);

  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / resolution, 0.0);
  vec3 near_pos_ws = PositionSS2WS(camera.view, camera.project, near_pos_ss);
  vec3 ray_dir = normalize(near_pos_ws - camera.pos_ws);

  vec4 sample_color = TracePath(camera.pos_ws, ray_dir);
  vec4 previous_color = imageLoad(canvas, pixel);
  float accumulation_weight = 1.0 / float(frame_num + 1);
  vec4 blended_color = frame_num == 0 ? sample_color
                                      : mix(previous_color, sample_color, accumulation_weight);
  imageStore(canvas, pixel, blended_color);
}
