# Параллельный поиск по файлам (pgrep)

## Начальная сборка

```sh
# Сборка
chmod +x build.sh
./build.sh
```

## Последующая сборка при изменениях

```sh
make -C build
```

## Запуск

```sh
# Пример запуска
cd build
pgrep -j 4 --regex --count "WARN|ERROR" "../benchmark/data"
```

```bash
(base) mirali777@109893531i:~/Projects/parallel_grep$ cd build
(base) mirali777@109893531i:~/Projects/parallel_grep/build$ pgrep -h

Usage:
 pgrep [options] <pattern>

Options:
 -d, --delimiter <string>  specify output delimiter
 -l, --list-name           list PID and process name
 -a, --list-full           list PID and full command line
 -v, --inverse             negates the matching
 -w, --lightweight         list all TID
 -c, --count               count of matching processes
 -f, --full                use full process name to match
 -g, --pgroup <PGID,...>   match listed process group IDs
 -G, --group <GID,...>     match real group IDs
 -i, --ignore-case         match case insensitively
 -n, --newest              select most recently started
 -o, --oldest              select least recently started
 -O, --older <seconds>     select where older than seconds
 -P, --parent <PPID,...>   match only child processes of the given parent
 -s, --session <SID,...>   match session IDs
 -t, --terminal <tty,...>  match by controlling terminal
 -u, --euid <ID,...>       match by effective IDs
 -U, --uid <ID,...>        match by real IDs
 -x, --exact               match exactly with the command name
 -F, --pidfile <file>      read PIDs from file
 -L, --logpidfile          fail if PID file is not locked
 -r, --runstates <state>   match runstates [D,S,Z,...]
 --ns <PID>                match the processes that belong to the same
                           namespace as <pid>
 --nslist <ns,...>         list which namespaces will be considered for
                           the --ns option.
                           Available namespaces: ipc, mnt, net, pid, user, uts

 -h, --help     display this help and exit
 -V, --version  output version information and exit

For more details see pgrep(1).
```

## Бенчмарки

### Генерация датасета для бенчмарков

```sh
# Запуск генератора датасетов
cd benchmark
python3 dataset_generator.py
```

### Запуск бенчмарков

```sh
# Сборка
cd benchmark
chmod +x build.sh
./build.sh
# Установка утилиты hyperfine (если ещё не установлена)
sudo apt-get install hyperfine
# Запуск
./run.sh
```

### Результаты бенчмарков

Сравнение запусков с 1, 4 и 6 потоками.

```bash
(base) mirali777@109893531i:~/Projects/parallel_grep/benchmark$ ./run.sh 

==============================
REGEX dataset=small pattern=ERROR|WARN jobs=1 4 6
==============================
Benchmark 1: ./build-release/pgrep --count --jobs 1 --regex "ERROR|WARN" "data/small" > /dev/null
  Time (mean ± σ):     800.6 ms ±  17.5 ms    [User: 777.6 ms, System: 326.2 ms]
  Range (min … max):   775.7 ms … 821.8 ms    10 runs
 
Benchmark 2: ./build-release/pgrep --count --jobs 4 --regex "ERROR|WARN" "data/small" > /dev/null
  Time (mean ± σ):     302.5 ms ±  14.3 ms    [User: 1019.8 ms, System: 354.7 ms]
  Range (min … max):   287.5 ms … 338.3 ms    10 runs
 
Benchmark 3: ./build-release/pgrep --count --jobs 6 --regex "ERROR|WARN" "data/small" > /dev/null
  Time (mean ± σ):     359.2 ms ±  15.5 ms    [User: 1269.1 ms, System: 426.4 ms]
  Range (min … max):   338.4 ms … 387.2 ms    10 runs
 
Summary
  './build-release/pgrep --count --jobs 4 --regex "ERROR|WARN" "data/small" > /dev/null' ran
    1.19 ± 0.08 times faster than './build-release/pgrep --count --jobs 6 --regex "ERROR|WARN" "data/small" > /dev/null'
    2.65 ± 0.14 times faster than './build-release/pgrep --count --jobs 1 --regex "ERROR|WARN" "data/small" > /dev/null'

==============================
LITERAL dataset=small pattern=TODO jobs=1 4 6
==============================
Benchmark 1: ./build-release/pgrep --count --jobs 1 "TODO" "data/small" > /dev/null
  Time (mean ± σ):     342.6 ms ±  18.0 ms    [User: 304.7 ms, System: 309.6 ms]
  Range (min … max):   310.7 ms … 369.8 ms    10 runs
 
Benchmark 2: ./build-release/pgrep --count --jobs 4 "TODO" "data/small" > /dev/null
  Time (mean ± σ):     322.7 ms ±  47.7 ms    [User: 589.9 ms, System: 387.4 ms]
  Range (min … max):   260.1 ms … 408.5 ms    10 runs
 
Benchmark 3: ./build-release/pgrep --count --jobs 6 "TODO" "data/small" > /dev/null
  Time (mean ± σ):     358.4 ms ±  31.9 ms    [User: 780.0 ms, System: 432.2 ms]
  Range (min … max):   313.4 ms … 422.7 ms    10 runs
 
Summary
  './build-release/pgrep --count --jobs 4 "TODO" "data/small" > /dev/null' ran
    1.06 ± 0.17 times faster than './build-release/pgrep --count --jobs 1 "TODO" "data/small" > /dev/null'
    1.11 ± 0.19 times faster than './build-release/pgrep --count --jobs 6 "TODO" "data/small" > /dev/null'

==============================
REGEX dataset=mix pattern=ERROR|WARN jobs=1 4 6
==============================
Benchmark 1: ./build-release/pgrep --count --jobs 1 --regex "ERROR|WARN" "data/mix" > /dev/null
  Time (mean ± σ):      1.966 s ±  0.088 s    [User: 1.948 s, System: 0.134 s]
  Range (min … max):    1.875 s …  2.187 s    10 runs
 
Benchmark 2: ./build-release/pgrep --count --jobs 4 --regex "ERROR|WARN" "data/mix" > /dev/null
  Time (mean ± σ):     595.4 ms ±  23.9 ms    [User: 2337.9 ms, System: 135.0 ms]
  Range (min … max):   575.4 ms … 651.4 ms    10 runs
 
Benchmark 3: ./build-release/pgrep --count --jobs 6 --regex "ERROR|WARN" "data/mix" > /dev/null
  Time (mean ± σ):     456.5 ms ±  32.7 ms    [User: 2664.3 ms, System: 161.9 ms]
  Range (min … max):   404.8 ms … 500.2 ms    10 runs
 
Summary
  './build-release/pgrep --count --jobs 6 --regex "ERROR|WARN" "data/mix" > /dev/null' ran
    1.30 ± 0.11 times faster than './build-release/pgrep --count --jobs 4 --regex "ERROR|WARN" "data/mix" > /dev/null'
    4.31 ± 0.36 times faster than './build-release/pgrep --count --jobs 1 --regex "ERROR|WARN" "data/mix" > /dev/null'

==============================
LITERAL dataset=mix pattern=TODO jobs=1 4 6
==============================
Benchmark 1: ./build-release/pgrep --count --jobs 1 "TODO" "data/mix" > /dev/null
  Time (mean ± σ):     157.1 ms ±  10.0 ms    [User: 124.2 ms, System: 117.0 ms]
  Range (min … max):   140.8 ms … 174.4 ms    10 runs
 
Benchmark 2: ./build-release/pgrep --count --jobs 4 "TODO" "data/mix" > /dev/null
  Time (mean ± σ):      78.0 ms ±   4.4 ms    [User: 172.9 ms, System: 128.0 ms]
  Range (min … max):    71.1 ms …  85.8 ms    10 runs
 
Benchmark 3: ./build-release/pgrep --count --jobs 6 "TODO" "data/mix" > /dev/null
  Time (mean ± σ):      86.6 ms ±  10.4 ms    [User: 247.9 ms, System: 153.9 ms]
  Range (min … max):    74.9 ms … 112.2 ms    10 runs
 
Summary
  './build-release/pgrep --count --jobs 4 "TODO" "data/mix" > /dev/null' ran
    1.11 ± 0.15 times faster than './build-release/pgrep --count --jobs 6 "TODO" "data/mix" > /dev/null'
    2.01 ± 0.17 times faster than './build-release/pgrep --count --jobs 1 "TODO" "data/mix" > /dev/null'

==============================
REGEX dataset=big pattern=ERROR|WARN jobs=1 4 6
==============================
Benchmark 1: ./build-release/pgrep --count --jobs 1 --regex "ERROR|WARN" "data/big" > /dev/null
  Time (mean ± σ):      1.263 s ±  0.026 s    [User: 1.257 s, System: 0.043 s]
  Range (min … max):    1.219 s …  1.296 s    10 runs
 
Benchmark 2: ./build-release/pgrep --count --jobs 4 --regex "ERROR|WARN" "data/big" > /dev/null
  Time (mean ± σ):     378.0 ms ±  17.5 ms    [User: 1448.5 ms, System: 43.3 ms]
  Range (min … max):   339.0 ms … 398.2 ms    10 runs
 
Benchmark 3: ./build-release/pgrep --count --jobs 6 --regex "ERROR|WARN" "data/big" > /dev/null
  Time (mean ± σ):     363.0 ms ±  13.9 ms    [User: 1410.6 ms, System: 41.3 ms]
  Range (min … max):   342.6 ms … 382.6 ms    10 runs
 
Summary
  './build-release/pgrep --count --jobs 6 --regex "ERROR|WARN" "data/big" > /dev/null' ran
    1.04 ± 0.06 times faster than './build-release/pgrep --count --jobs 4 --regex "ERROR|WARN" "data/big" > /dev/null'
    3.48 ± 0.15 times faster than './build-release/pgrep --count --jobs 1 --regex "ERROR|WARN" "data/big" > /dev/null'

==============================
LITERAL dataset=big pattern=TODO jobs=1 4 6
==============================
Benchmark 1: ./build-release/pgrep --count --jobs 1 "TODO" "data/big" > /dev/null
  Time (mean ± σ):      43.9 ms ±   3.5 ms    [User: 29.8 ms, System: 23.5 ms]
  Range (min … max):    37.0 ms …  47.8 ms    10 runs
 
Benchmark 2: ./build-release/pgrep --count --jobs 4 "TODO" "data/big" > /dev/null
  Time (mean ± σ):      15.2 ms ±   2.7 ms    [User: 32.5 ms, System: 21.5 ms]
  Range (min … max):    12.5 ms …  19.9 ms    10 runs
 
Benchmark 3: ./build-release/pgrep --count --jobs 6 "TODO" "data/big" > /dev/null
  Time (mean ± σ):      15.1 ms ±   2.8 ms    [User: 32.4 ms, System: 22.2 ms]
  Range (min … max):    12.6 ms …  20.3 ms    10 runs
 
Summary
  './build-release/pgrep --count --jobs 6 "TODO" "data/big" > /dev/null' ran
    1.01 ± 0.26 times faster than './build-release/pgrep --count --jobs 4 "TODO" "data/big" > /dev/null'
    2.91 ± 0.59 times faster than './build-release/pgrep --count --jobs 1 "TODO" "data/big" > /dev/null'
```
