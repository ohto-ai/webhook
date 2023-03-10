#!/usr/bin/env python

import requests
import platform

try:
    # For Python 3
    import distro
    system_version = '{}-{}'.format(distro.linux_distribution(full_distribution_name=False)[0], distro.linux_distribution(full_distribution_name=False)[1])
except ImportError:
    # For Python 2 or if distro module is not installed
    system_version = '{}-{}'.format(platform.linux_distribution()[0], platform.linux_distribution()[1])

# Replace with the project's owner and name
project_owner = 'Ohto-Ai'
project_name = 'webhook'

# API endpoint to get the latest release version
url = f'https://api.github.com/repos/{project_owner}/{project_name}/releases/latest'

# Make the API request
response = requests.get(url)

# Find the latest version of the release
latest_version = ''
latest_version_download_url = ''
for release in response.json()['assets']:
    version = release['name']
    if version.startswith('{}_{}'.format(project_name, system_version)):
        latest_version = version
        latest_version_download_url = release['browser_download_url']
        print('Find version', latest_version)
        break

if not latest_version:
    print('No version found')
else:   
    # Download the release
    print('Downloading', latest_version)
    download_response = requests.get(latest_version_download_url)

    # Save the file
    with open(project_name, 'wb') as f:
        f.write(download_response.content)
