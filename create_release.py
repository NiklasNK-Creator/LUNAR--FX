#!/usr/bin/env python3
"""
Create GitHub release with files
"""

import requests
import os
from pathlib import Path

GITHUB_TOKEN = "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"
REPO = "NiklasNK-Creator/LUNAR--FX"
TAG = "2026.04.24"
RELEASE_NAME = "Release 2026.04.24"
RELEASE_BODY = """LUNAR --FX Release

- LUNAR.SNITCH plugin: Let your edit shine like a moon
- CustomTkinter installer with modern dark theme UI
- Auto admin rights request
- AE 2020+ support with correct installation path

Installation:
1. Download LUNAR_FX_Installer.exe
2. Run as administrator
3. Select your After Effects version
4. Install
"""

def create_release():
    """Create GitHub release"""
    url = f"https://api.github.com/repos/{REPO}/releases"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }
    data = {
        "tag_name": TAG,
        "target_commitish": "master",
        "name": RELEASE_NAME,
        "body": RELEASE_BODY,
        "draft": False,
        "prerelease": False
    }
    
    response = requests.post(url, json=data, headers=headers)
    if response.status_code == 201:
        release_data = response.json()
        print(f"Release created: {release_data['html_url']}")
        return release_data['upload_url']
    else:
        print(f"Failed to create release: {response.text}")
        return None

def upload_file(upload_url, file_path, content_type):
    """Upload file to release"""
    file_name = os.path.basename(file_path)
    upload_url = upload_url.replace("{?name,label}", f"?name={file_name}")
    
    with open(file_path, 'rb') as f:
        headers = {
            "Authorization": f"token {GITHUB_TOKEN}",
            "Content-Type": content_type
        }
        response = requests.post(upload_url, data=f, headers=headers)
        
    if response.status_code == 201:
        print(f"Uploaded: {file_name}")
    else:
        print(f"Failed to upload {file_name}: {response.text}")

def main():
    print("Creating GitHub release...")
    upload_url = create_release()
    
    if upload_url:
        print("\nUploading files...")
        installer_path = Path(__file__).parent / "Build" / "LUNAR_FX_Installer.exe"
        plugin_path = Path(__file__).parent / "plugins" / "LUNAR_FX.aex"
        
        if installer_path.exists():
            upload_file(upload_url, str(installer_path), "application/x-msdownload")
        
        if plugin_path.exists():
            upload_file(upload_url, str(plugin_path), "application/octet-stream")
        
        print("\nRelease complete!")

if __name__ == "__main__":
    main()
