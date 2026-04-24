import os
import sys
import subprocess
import requests
from pathlib import Path

GITHUB_TOKEN = os.environ.get('GITHUB_TOKEN') or (sys.argv[4] if len(sys.argv) > 4 else None)
REPO = 'NiklasNK-Creator/LUNAR--FX'

def run_command(cmd, cwd=None):
    result = subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
        return False
    print(result.stdout)
    return True

def has_changes(cwd):
    result = subprocess.run('git status --porcelain', shell=True, cwd=cwd, capture_output=True, text=True)
    return bool(result.stdout.strip())

def create_release(tag, title, notes, asset_path):
    if not GITHUB_TOKEN:
        print("GITHUB_TOKEN environment variable not set")
        return False
    
    url = f"https://api.github.com/repos/{REPO}/releases"
    headers = {
        'Authorization': f'token {GITHUB_TOKEN}',
        'Accept': 'application/vnd.github.v3+json'
    }
    
    data = {
        'tag_name': tag,
        'name': title,
        'body': notes,
        'draft': False,
        'prerelease': False
    }
    
    response = requests.post(url, headers=headers, json=data)
    if response.status_code != 201:
        print(f"Failed to create release: {response.text}")
        return False
    
    release_id = response.json()['id']
    
    upload_url = f"https://uploads.github.com/repos/{REPO}/releases/{release_id}/assets?name={Path(asset_path).name}"
    headers['Content-Type'] = 'application/octet-stream'
    
    with open(asset_path, 'rb') as f:
        response = requests.post(upload_url, headers=headers, data=f)
    
    if response.status_code != 201:
        print(f"Failed to upload asset: {response.text}")
        return False
    
    return True

def main():
    root = Path(__file__).parent.parent
    
    tag = sys.argv[1] if len(sys.argv) > 1 else 'v1.1.0'
    title = sys.argv[2] if len(sys.argv) > 2 else f'{tag} - LUNAR.IMP'
    notes = sys.argv[3] if len(sys.argv) > 3 else 'Add LUNAR.IMP effect with impact frame capture and radial expansion from center with 7 edge types'
    
    print(f"Creating release {tag}...")
    
    if has_changes(root):
        if not run_command('git add -A', cwd=root):
            return
        if not run_command(f'git commit -m "Release {tag}"', cwd=root):
            return
        if not run_command('git pull --rebase', cwd=root):
            return
        if not run_command('git push', cwd=root):
            return
    else:
        print("No changes to commit")
    
    asset_path = root / 'dist' / 'LUNAR_FX_Installer.exe'
    if not asset_path.exists():
        print(f"Installer not found at {asset_path}")
        return
    
    if create_release(tag, title, notes, str(asset_path)):
        print(f"Release {tag} created successfully!")
    else:
        print("Failed to create release")

if __name__ == '__main__':
    main()
