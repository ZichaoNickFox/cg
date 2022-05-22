import os
import pathlib

dirs = ["playground", "engine"]
exts = [".cc", ".h", ".cpp", ".hpp", ".glsl"]

all_file = []
for dir in dirs:
  for root, subdirs, files in os.walk("engine"):
    for file in files:
      all_file.append(root + "\\" + file)

file_num = 0;
line_num = 0;
for file in all_file:
  ext = pathlib.Path(file).suffix
  if ext in exts:
    file_num = file_num + 1

    f = open(file, "r")
    line_count = 0
    for line in file:
        if line != "\n":
            line_count += 1
    f.close()
    line_num = line_num + line_count

print("file_num", file_num)
print("line_num", line_num)
