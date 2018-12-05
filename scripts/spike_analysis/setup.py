from setuptools import setup,find_packages

setup(
    name='spike_analysis',
    version='0.1',
    packages=['spike_accuracy','analysis_utils','traffic_analysis'],
    #packages=find_packages(),
    include_package_data=True,
    install_requires=["py-flags","Click","dask",'pandas','numpy','sqlalchemy','tqdm','click_spinner'],
    entry_points="""
    [console_scripts]
    spike_accuracy=spike_accuracy.spike_comps:compare_nscs_nemo
    """,

    author='Mark Plagge',
    author_email='plaggm@rpi.edu',
    description=''
)
