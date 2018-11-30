import setuptools

with open('README.md', 'r') as fh:
    long_description = fh.read()

setuptools.setup(
    name="traffic_analysis",
    version="0.0.1",
    author="Mark Plagge",
    description="NeMo Traffic Analysis",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=setuptools.find_packages()
)