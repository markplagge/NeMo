from setuptools import setup,find_packages

setup(
	name='NeMoScriptTools',
	version='0.0.1',
	py_modules=['NeMo_TN', 'tn_api'],
	install_requires=[
		'Click',
		'tqdm',
		'numba',
		'jsoncomment',
		'numpy',
	],
	packages=find_packages(),
	package_data={
		'' : ['*.json', '*.sfti']
	},


	entry_points='''
	[console_scripts]
	NeMo_TN:NeMo_TN:cli
	'''
)