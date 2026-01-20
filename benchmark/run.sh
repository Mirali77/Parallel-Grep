hyperfine -w 2 -r 10 \
  './build-release/pgrep --count --jobs 1 --regex "ERROR|WARN" data/mix > /dev/null' \
  './build-release/pgrep --count --jobs 4 --regex "ERROR|WARN" data/mix > /dev/null' \
  './build-release/pgrep --count --jobs 12 --regex "ERROR|WARN" data/mix > /dev/null'