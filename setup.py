from setuptools import setup, Extension

with open("README.md", encoding="utf-8") as f:
    long_desc = f.read()

setup(
    name="qbase58",
    version="1.0.1",
    author="Quick Vectors",
    author_email="felipe@qvecs.com",
    description="Quick Base58 encoding/decoding.",
    long_description=long_desc,
    long_description_content_type="text/markdown",
    url="https://github.com/qvecs/qbase58",
    project_urls={
        "Bug Tracker": "https://github.com/qvecs/qbase58/issues",
        "Source Code": "https://github.com/qvecs/qbase58",
    },
    license="MIT",
    python_requires=">=3.8",
    include_package_data=True,
    packages=["qbase58"],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C",
        "Programming Language :: Python :: Implementation :: CPython",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Topic :: Utilities",
    ],
    ext_modules=[
        Extension(
            name="qbase58.py_base58",
            sources=[
                "qbase58/base58.c",
                "qbase58/py_base58.c",
            ],
            include_dirs=["qbase58"],
            extra_compile_args=[
                "-std=gnu17",
                "-Ofast",
                "-flto",
                "-fomit-frame-pointer",
                "-funroll-loops",
                "-ffast-math",
                "-fstrict-aliasing",
            ],
        )
    ],
    extras_require={"dev": ["pytest"]},
)
