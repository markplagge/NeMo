from setuptools import setup,find_packages

setup(
    name='spike_validation_tools',
    version='0.1',
    packages=['spike_validation_tools','spike_comps','spike_validation', 'spike_validation.file_load'],
    #packages=find_packages(),
    include_package_data=True,
    install_requires=["Click","dask",],
    entry_points="""
    [console_scripts]
    spike_comps=spike_comps:compare_nscs_nemo
    """,

    author='plaggm',
    author_email='',
    description=''
)
