from typing import Union

def encode(data: bytes) -> str:
    """
    Encode raw bytes into a Base58 bytes object.

    Args:
        data: Raw bytes to encode.

    Returns:
        Encoded Base58 bytes object.
    """
    ...

def decode(data: Union[bytes, str]) -> bytes:
    """
    Decode Base58 data (bytes or str) into raw bytes.

    Raises:
        ValueError: If the data includes invalid Base58 characters.

    Args:
        data: Base58 data to decode.

    Returns:
        Decoded raw bytes.
    """
    ...
