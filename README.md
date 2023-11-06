# jetbrains-perf-investigation
Test task solution for JetBrains PerformanceInvestigationCpp.

## Work done
- Optimization of initial solution done;
- Benchmark tool (+ calibration) is created;
- Performance analysis steps described in a report;

## Installation
Benchmark requires:
[Python 3](https://www.python.org/) v3.9 to run.
Install Python dependencies:
```
pip install -r requirements.txt
```
[GNU Time](https://www.gnu.org/software/time/) to measure process metrics (e.g. CPU/Memory utilization):
```
brew install gnu-time
```

## Benchmark features
- adaptable config
- warmup
- multiple iterations benchmark
- process timeout handling
- execution time measurement (using Python [time.perf_counter()](https://docs.python.org/3/library/time.html))
- Memory / CPU measurement (using [GNU Time](https://www.gnu.org/software/time/))
- summary reporting: csv and graph (with comparison between multiple binaries benchmarked)

### Benchmark calibration
Investigation into dependency of measurement precision on: 
- number of iterations
- time measurement function
-  number of warmup iterations
-  gtime overhead 

is provided at [benchmark calibration report](https://github.com/yurysup/jetbrains-perf-investigation/blob/main/analysis/Benchmark_calibration.docx).

### Execution

Set configurations in `benchmark_conf.json` [file](https://github.com/yurysup/jetbrains-perf-investigation/blob/main/benchmark_conf.json).

Execute `benchmark.py` [file](https://github.com/yurysup/jetbrains-perf-investigation/blob/main/benchmark.py).
```
python3 benchmark.py
```

Report will be generated at `/reports` dir:

- report.csv (summry metrics)
- comparison.png (graphs with exec time, memory & cpu utilization)

Human-readable output is provided in console.

> :warning: **_NOTE:_**  Check for timeout messages as timedout processes won't be added to report (e.g. Timeout reached for PerformanceInvestigationCpp_v0 with maxPrime = 1000000. Skipping...).

## Performance analysis & optimization

[Optimization report](https://github.com/yurysup/jetbrains-perf-investigation/blob/main/analysis/Optimization.docx) added at `/analysis` dir with benchmark and profiling results.

Issues found in original solution are added as comments to [main_original.cpp](https://github.com/yurysup/jetbrains-perf-investigation/blob/main/archive/main_original.cpp).

### Versions benchmarked

All compiled binaries can be found at `/bin`.
All sources (benchmarked app versions) can be found at `/archive`.
Final implementation is located at `main.cpp`.
