import random

import pytest

import qbase58


def test_basic_encode_decode():
    original = b"Hello Base58!"
    encoded = qbase58.encode(original)
    decoded = qbase58.decode(encoded)
    assert decoded == original, f"Decoded data differs from original: {decoded} != {original}"


def test_empty():
    data = b""
    encoded = qbase58.encode(data)
    decoded = qbase58.decode(encoded)
    assert decoded == data, "Empty input should encode/decode back to empty bytes."


def test_leading_zeros():
    data = b"\x00\x00\x00ABC"
    encoded = qbase58.encode(data)
    decoded = qbase58.decode(encoded)
    assert decoded == data, f"Leading zeros not preserved: {decoded} != {data}"


def test_invalid_characters():
    invalid_string = b"0OIl"
    with pytest.raises(ValueError):
        _ = qbase58.decode(invalid_string)


def test_random_binary():
    data = bytes(random.getrandbits(8) for _ in range(10_000))
    encoded = qbase58.encode(data)
    decoded = qbase58.decode(encoded)
    assert decoded == data, "Random binary data failed to round-trip."
