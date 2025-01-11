import qbase58


def test_readme():
    string = b"Hello, World!"

    encoded = qbase58.encode(string)
    print(f"Encoded: {encoded}")

    decoded = qbase58.decode(encoded)
    print(f"Decoded: {decoded}")

    assert decoded == string, f"Decoded data differs from original: {decoded} != {string}"
