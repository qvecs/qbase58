"""
Testing the following libraries:

1) qbase58
2) base58   (https://pypi.org/project/base58/)
3) based58  (https://pypi.org/project/based58/)

We test on three different string (byte) sizes:
    1) Small (100 bytes)
    2) Medium (1,000 bytes)
    3) Large (5,000 bytes)

To run:
    virtualenv .venv
    source .venv/bin/activate
    pip install qbase58 base58 based58
    python scripts/speedtest.py
"""

import time
import statistics
import random

random.seed(0)

SMALL_DATA = bytes(random.getrandbits(8) for _ in range(100))
MEDIUM_DATA = bytes(random.getrandbits(8) for _ in range(1_000))
LARGE_DATA = bytes(random.getrandbits(8) for _ in range(5_000))

import base58
import based58
import qbase58


def base58_serialize_deserialize(data: bytes, num_iterations: int) -> None:
    for _ in range(num_iterations):
        encoded = base58.b58encode(data)
        _ = base58.b58decode(encoded)


def based58_rust_serialize_deserialize(data: bytes, num_iterations: int) -> None:
    for _ in range(num_iterations):
        encoded = based58.b58encode(data)
        _ = based58.b58decode(encoded)


def qbase58_serialize_deserialize(data: bytes, num_iterations: int) -> None:
    for _ in range(num_iterations):
        encoded = qbase58.encode(data)
        _ = qbase58.decode(encoded)


def time_library(label: str, func, data: bytes, num_iterations: int) -> float:
    t0 = time.perf_counter()
    func(data, num_iterations)
    t1 = time.perf_counter()
    return t1 - t0


def run_benchmark(num_iterations_small=10000, num_iterations_medium=5000, num_iterations_large=1000, num_repeats=5):
    data_sets = [
        ("SMALL", SMALL_DATA, num_iterations_small),
        ("MEDIUM", MEDIUM_DATA, num_iterations_medium),
        ("LARGE", LARGE_DATA, num_iterations_large),
    ]
    libs = [
        ("base58", base58_serialize_deserialize),
        ("based58", based58_rust_serialize_deserialize),
        ("qbase58", qbase58_serialize_deserialize),
    ]

    print("==== Base58 Encode+Decode Benchmark ====\n")

    results = []
    for data_label, data_obj, iterations in data_sets:
        print(f"--- Data Set: {data_label} (size={len(data_obj)} bytes) ---")
        for lib_label, func in libs:
            timings = []
            for _ in range(num_repeats):
                duration = time_library(lib_label, func, data_obj, iterations)
                timings.append(duration)
            avg_time = statistics.mean(timings)
            sd_time = statistics.pstdev(timings)

            # Convert seconds -> milliseconds
            avg_ms = avg_time * 1000
            sd_ms = sd_time * 1000
            print(f"{lib_label:8s} => avg: {avg_ms:.4f} ms (std: {sd_ms:.4f} ms) over {iterations} iterations")

        print()


if __name__ == "__main__":
    run_benchmark(
        num_iterations_small=1000,
        num_iterations_medium=500,
        num_iterations_large=10,
        num_repeats=5,
    )
