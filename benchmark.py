import subprocess
import time
import re
import csv
import json
import numpy as np
from functools import wraps


def read_config() -> dict:
    with open("benchmark_conf.json", "r") as file:
        return json.load(file)


def get_gtime_measurements(gtime_output: str) -> dict:
    patterns = {
        "memory": "Maximum resident set size \(kbytes\): (\d+)",
        "cpu_pct": "Percent of CPU this job got: (\d+)%",
    }
    measurements = {}
    for measurement, pattern in patterns.items():
        match = re.search(pattern=pattern, string=gtime_output)
        if match:
            measurements[measurement] = int(match.group(1))
    return measurements


def write_csv_report(aggregated_data, filename="reports/report.csv"):
    with open(filename, "w", newline="") as csvfile:
        fieldnames = [
            "test_label",
            "max_prime",
            "average_execution_time",
            "average_memory",
            "average_cpu_pct",
        ]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for data_dict in aggregated_data:
            writer.writerow(data_dict)


def timing_decorator(function):
    @wraps(function)
    def wrapper(*args, **kwargs):
        start_time = time.perf_counter()
        result = function(*args, **kwargs)
        end_time = time.perf_counter()
        elapsed_time = end_time - start_time
        return result, elapsed_time

    return wrapper


@timing_decorator
def exec_process(binary: str, max_prime: int, timeout: int) -> bytes:
    result = subprocess.check_output(
        f"gtime -v ./bin/{binary} {max_prime} > /dev/null",  # Redirect only ./main output to /dev/null
        stderr=subprocess.STDOUT,
        shell=True,
        timeout=timeout,
    )
    return result


# Extract values from the configuration
config = read_config()
max_primes = config["max_primes"]
iterations = config["benchmark"]["iterations"]
timeout = config["benchmark"]["timeout_seconds"]
iterations_warmup = config["warmup"]["iterations"]
timeout_warmup = config["warmup"]["timeout_seconds"]
binaries = config["binaries"]


aggregated_data = []

for binary in binaries:
    for max_prime in max_primes:
        failed = False
        time_measurements = []
        memory_measurements = []
        cpu_measurements = []

        for _ in range(iterations_warmup):
            try:
                result_bytes, exec_time = exec_process(
                    binary=binary, max_prime=max_prime, timeout=timeout_warmup
                )
                result = result_bytes.decode("utf-8")
            except subprocess.TimeoutExpired:
                failed = True
                print(
                    f"Timeout reached for {binary} with maxPrime = {max_prime}. Skipping..."
                )
                break
            except subprocess.CalledProcessError as e:
                failed = True
                print(e)
                print(
                    f"Something went wrong while executing subprocess {binary}: check gtime installation or the compiled executable."
                )
                break

        if not failed:
            for _ in range(iterations):
                try:
                    result_bytes, exec_time = exec_process(
                        binary=binary, max_prime=max_prime, timeout=timeout
                    )
                    result = result_bytes.decode("utf-8")
                    measurements = get_gtime_measurements(result)
                    time_measurements.append(exec_time)
                    memory_measurements.append(measurements["memory"])
                    cpu_measurements.append(measurements["cpu_pct"])
                except subprocess.TimeoutExpired:
                    failed = True
                    print(
                        f"Timeout reached for {binary} with maxPrime = {max_prime}. Skipping..."
                    )
                    break
                except subprocess.CalledProcessError as e:
                    failed = True
                    print(e)
                    print(
                        f"Something went wrong while executing subprocess {binary}: check gtime installation or the compiled executable."
                    )
                    break

        if not failed:
            """
            average_time = total_time / iterations
            average_memory = total_memory / iterations
            average_cpu_pct = total_cpu_pct / iterations
            aggregated_data.append(
                {
                    'test_label': binaries,
                    'max_prime': max_prime,
                    'average_execution_time': average_time,
                    'average_memory': average_memory,
                    'average_cpu_pct': average_cpu_pct,
                }
            )
            """
            std_dev_time = np.std(time_measurements)
            mean_time = np.mean(time_measurements)
            mean_memory = np.mean(memory_measurements)
            mean_cpu = np.mean(cpu_measurements)
            print(f"For {binary} with maxPrime = {max_prime}:")
            print(f"    Average execution time: {mean_time:.4f}s")
            print(f"    Average memory used: {mean_memory:.2f} kbytes")
            print(f"    Average CPU used: {mean_cpu:.1f} %")
            print(f"    Time measurement deviation: {std_dev_time}")

# print(aggregated_data)
# write_csv_report(aggregated_data)
