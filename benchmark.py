import subprocess
import time
import re
import csv
import json


def read_config() -> dict:
    with open("benchmark_conf.json", "r") as file:
        return json.load(file)


def get_gtime_measurement(measurement: str, gtime_output: str) -> int:
    patterns = {
        "memory": "Maximum resident set size \(kbytes\): (\d+)",
        "cpu_pct": "Percent of CPU this job got: (\d+)%",
    }
    match = re.search(pattern=patterns[measurement], string=gtime_output)
    if match:
        return int(match.group(1))
    else:
        return 0


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


# Extract values from the configuration
config = read_config()
max_primes = config["max_primes"]
iterations = config["iterations"]
timeout_seconds = config["timeout_seconds"]
binary = config["binary"]

aggregated_data = []
for max_prime in max_primes:
    total_time = 0
    total_memory = 0
    total_cpu_pct = 0
    failed = False

    # warmup required
    for _ in range(iterations):
        try:
            start_time = time.time()
            result_bytes = subprocess.check_output(
                f"gtime -v ./bin/{binary} {max_prime} > /dev/null",  # Redirect only ./main output to /dev/null
                stderr=subprocess.STDOUT,
                shell=True,
                timeout=timeout_seconds,
            )
            end_time = time.time()

            result = result_bytes.decode("utf-8")

            total_memory += get_gtime_measurement("memory", result)
            total_cpu_pct += get_gtime_measurement("cpu_pct", result)
            total_time += end_time - start_time

        except subprocess.TimeoutExpired:
            failed = True
            print(f"Timeout reached for maxPrime = {max_prime}. Skipping...")
            break
        except subprocess.CalledProcessError:
            failed = True
            print(
                f"Something went wrong while executing benchmark: check gtime installation or a path to compiled executable."
            )
            break

    if not failed:
        average_time = total_time / iterations
        average_memory = total_memory / iterations
        average_cpu_pct = total_cpu_pct / iterations
        aggregated_data.append(
            {
                "test_label": binary,
                "max_prime": max_prime,
                "average_execution_time": average_time,
                "average_memory": average_memory,
                "average_cpu_pct": average_cpu_pct,
            }
        )
        # print(f"For maxPrime = {max_prime}:")
        # print(f"    Average execution time: {average_time:.4f} seconds")
        # print(f"    Average memory used: {average_memory:.2f} kbytes")
        # print(f"    Average CPU used: {average_cpu_pct:.2f} %")

print(aggregated_data)
write_csv_report(aggregated_data)
