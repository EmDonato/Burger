from setuptools import find_packages
from setuptools import setup

setup(
    name='stm32_nucleo_f303re_driver',
    version='0.1.0',
    packages=find_packages(
        include=('stm32_nucleo_f303re_driver', 'stm32_nucleo_f303re_driver.*')),
)
