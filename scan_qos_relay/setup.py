from setuptools import setup

package_name = 'scan_qos_relay'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='arpan',
    maintainer_email='your_email@example.com',
    description='Relay node to convert /scan QoS from Best Effort to Reliable',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'scan_qos_relay = scan_qos_relay.relay_node:main'
        ],
    },
)

