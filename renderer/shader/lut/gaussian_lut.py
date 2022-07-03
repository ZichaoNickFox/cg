# %% gaussian.glsl
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as stats

MAX_RADIUS = 10
N_PER_RADIUS = int(1 / 0.1)
N_PER_LUT_ROW = MAX_RADIUS * N_PER_RADIUS 
N_SIGMA = 3

def pdfs(mu, sigma, n_sigma, N):
  x = np.linspace(0, mu + n_sigma * sigma, N)
  return stats.norm.pdf(x, mu, sigma)

def gen_lut_row(sample_radius):
  mu = 0
  sigma = sample_radius / 5
  n_sigma = N_SIGMA
  N = sample_radius * N_PER_RADIUS
  return pdfs(mu, sigma, n_sigma, N)

LUT = np.zeros((MAX_RADIUS, N_PER_LUT_ROW))
for sample_radius in range(1, MAX_RADIUS):
  lut_row = np.resize(gen_lut_row(sample_radius), N_PER_LUT_ROW)
  for i in range(sample_radius * N_PER_RADIUS, N_PER_LUT_ROW):
    lut_row[i] = 0
  LUT[sample_radius] = lut_row

# formating : https://realpython.com/python-string-formatting/
with open("gaussian_lut.glsl", "w") as file:
  file.write(f"const float gaussian_lut[{MAX_RADIUS}][{N_PER_LUT_ROW}] = ");
  file.write("{\n")
  for i in range(0, MAX_RADIUS):
    file.write("  {");
    for j in range(0, N_PER_LUT_ROW):
      file.write(f"{LUT[i, j]:.3f}")
      file.write(",")
    file.write("},\n")
  file.write("};");

#%% test
def draw_lut_row(sample_radius):
  x = np.linspace(0, sample_radius, sample_radius * N_PER_RADIUS)
  indices = range(0, sample_radius * N_PER_RADIUS)
  plt.plot(x, LUT[sample_radius, indices])
  plt.show()
draw_lut_row(1)

#%% sample.glsl
def gaussian_pdf(sample_radius : int, distance):
  clamp = lambda a, b, v : min(max(a, v), b)
  sample_radius = clamp(1, MAX_RADIUS - 1, sample_radius);
  index = min(sample_radius, distance) * N_PER_RADIUS;
  return LUT[sample_radius, index]

gaussian_pdf(1, 1.1)
