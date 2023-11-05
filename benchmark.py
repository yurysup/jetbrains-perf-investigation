import subprocess
import time
import re
import csv
import json
import os
import signal
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


def exec_process(binary: str, max_prime: int):
    return subprocess.Popen(
        ["gtime", "-v", f"./bin/{binary}", str(max_prime)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        preexec_fn=os.setsid,
    )


def check_binaries(binaries: list):
    # Check if all the required binaries exist in the specified bin directory.
    # Check if bin_dir is actually a directory
    bin_dir = "./bin"
    if not os.path.isdir(bin_dir):
        print(f"The directory {bin_dir} does not exist.")
        return False

    missing_binaries = []
    # Iterate over the list of binaries and check each one
    for binary in binaries:
        binary_path = os.path.join(bin_dir, binary)
        if not os.path.isfile(binary_path):
            missing_binaries.append(binary)

    if missing_binaries:
        print(f"The following binaries are missing: {', '.join(missing_binaries)}")
        return False
    return True


# Extract values from the configuration
config = read_config()
max_primes = config["max_primes"]
iterations = config["benchmark"]["iterations"]
timeout = config["benchmark"]["timeout_seconds"]
iterations_warmup = config["warmup"]["iterations"]
timeout_warmup = config["warmup"]["timeout_seconds"]
binaries = config["binaries"]


if not check_binaries(binaries=binaries):
    exit(-1)


aggregated_data = []
for binary in binaries:
    failed = False
    for max_prime in max_primes:
        if failed:
            break
        time_measurements = []
        memory_measurements = []
        cpu_measurements = []

        for _ in range(iterations_warmup):
            try:
                process = exec_process(binary=binary, max_prime=max_prime)
                process.communicate(timeout=timeout_warmup)
            except subprocess.TimeoutExpired:
                failed = True
                # Send the SIGTERM signal to the process group to terminate all processes in the group
                os.killpg(os.getpgid(process.pid), signal.SIGINT)
                time.sleep(1)
                process.communicate()
                print(
                    f"Timeout (warmup) reached for {binary} with maxPrime = {max_prime}. Skipping..."
                )
                break
            except FileNotFoundError as e:
                failed = True
                print(e)
                print(
                    f"Something went wrong while executing subprocess {binary}: check gtime installation."
                )
                break

        if not failed:
            for _ in range(iterations):
                try:
                    start_time = time.perf_counter()
                    process = exec_process(binary=binary, max_prime=max_prime)
                    stdout_data, stderr_data = process.communicate(timeout=timeout)
                    exec_time = time.perf_counter() - start_time
                    result = stderr_data.decode("utf-8")
                    measurements = get_gtime_measurements(result)
                    time_measurements.append(exec_time)
                    memory_measurements.append(measurements["memory"])
                    cpu_measurements.append(measurements["cpu_pct"])
                except subprocess.TimeoutExpired:
                    failed = True
                    # Send the SIGTERM signal to the process group to terminate all processes in the group
                    os.killpg(os.getpgid(process.pid), signal.SIGINT)
                    time.sleep(1)
                    process.communicate()
                    print(
                        f"Timeout reached for {binary} with maxPrime = {max_prime}. Skipping..."
                    )
                    break
                except FileNotFoundError as e:
                    failed = True
                    print(e)
                    print(
                        f"Something went wrong while executing subprocess {binary}: check gtime installation."
                    )
                    break

        if not failed:
            std_dev_time = np.std(time_measurements)
            mean_time = np.mean(time_measurements)
            mean_memory = np.mean(memory_measurements)
            mean_cpu = np.mean(cpu_measurements)
            aggregated_data.append(
                {
                    "test_label": binary,
                    "max_prime": max_prime,
                    "average_execution_time": mean_time,
                    "average_memory": mean_memory,
                    "average_cpu_pct": mean_cpu,
                }
            )
            print(f"For {binary} with maxPrime = {max_prime}:")
            print(
                f"    Average execution time: {mean_time:.4f}s (+/-{std_dev_time:.4f}s)"
            )
            print(f"    Average memory used: {mean_memory:.0f} kbytes")
            print(f"    Average CPU used: {mean_cpu:.0f} %")

write_csv_report(aggregated_data)
