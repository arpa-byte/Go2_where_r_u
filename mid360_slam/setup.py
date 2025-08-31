import os
from glob import glob
from setuptools import find_packages, setup

package_name = 'mid360_slam'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        # --- THIS IS THE CRITICAL FIX ---
        # This part tells colcon to find all files in the 'launch' directory
        # and install them to the correct location in the 'install' space.
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*launch.[pxy][yma]*'))),
        
        # This line does the same for your config file.
        (os.path.join('share', package_name, 'config'), glob(os.path.join('config', '*.yaml'))),
        
        #Installing the rviz config file
        (os.path.join('share', package_name, 'config'), glob(os.path.join('config', '*.rviz'))),

    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='arpan',
    maintainer_email='arpanekka.a1.73@gmail.com',
    description='Launch and configuration files for MID360 SLAM.',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'relay_topics = mid360_slam.relay_topics:main',
        ],
    },
)