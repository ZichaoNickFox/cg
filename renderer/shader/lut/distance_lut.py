#%%
import math

RANGE = 100

# formating : https://realpython.com/python-string-formatting/
with open("distance_lut.glsl", "w") as file:
  file.write(f"const float distance_lut[{RANGE}][{RANGE}] = ");
  file.write("{\n")
  for row in range(0, RANGE):
    file.write("  {");
    for col in range(0, RANGE):
      distance = math.sqrt(row * row + col * col)
      file.write(f"{distance:.3f}")
      file.write(",")
    file.write("},\n")
  file.write("};");
# %%
