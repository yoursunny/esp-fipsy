#!/bin/bash
set -euo pipefail
cd "$( dirname "${BASH_SOURCE[0]}" )"

mkdir -p ./epoxyfsdata

curl -fsLS -o ./epoxyfsdata/v1-blinky1.jed \
  https://github.com/MoCoMakers/Fipsy-FPGA-edu/raw/a18f0a2c2f694e6ed9b4d86fd7faf1e185687a1f/Examples/FipsyV1%20-%20X02-256/1.%20Blinky/project_files/Implementation/FipsyBaseline_Implementation.jed

curl -fsLS -o ./epoxyfsdata/v1-blinky2.jed \
  https://github.com/MoCoMakers/Fipsy-FPGA-edu/raw/a18f0a2c2f694e6ed9b4d86fd7faf1e185687a1f/Examples/FipsyV1%20-%20X02-256/2.%20Blinky%202HZ/project_files/Implementation/FipsyBaseline_Implementation.jed

curl -fsLS -o ./epoxyfsdata/v2-blinky1.jed \
  https://github.com/MoCoMakers/Fipsy-FPGA-edu/raw/a18f0a2c2f694e6ed9b4d86fd7faf1e185687a1f/Examples/FipsyV2%20-%20XO2-1200/1.%20Blinky/project_files/Implementation/FipsyBaseline_Implementation.jed

curl -fsLS -o ./epoxyfsdata/v2-blinky2.jed \
  https://github.com/MoCoMakers/Fipsy-FPGA-edu/raw/a18f0a2c2f694e6ed9b4d86fd7faf1e185687a1f/Examples/FipsyV2%20-%20XO2-1200/2.%20Blinky%202HZ/project_files/Implementation/FipsyBaseline_Implementation.jed
