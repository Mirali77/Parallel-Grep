set -euo pipefail

cmake -S .. -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
python3 dataset_generator.py
