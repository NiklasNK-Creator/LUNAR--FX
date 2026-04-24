#!/usr/bin/env python3
"""
LUNAR --FX Automated Release Script
Builds plugin, installer, and creates GitHub release
"""

import os
import sys
import subprocess
from pathlib import Path
import shutil

def build_plugin():
    """Build the LUNAR.SNITCH plugin"""
    print("Building LUNAR.SNITCH plugin...")
    msbuild_path = r"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
    project_path = Path(__file__).parent / "Source" / "LUNAR_SNITCH" / "LUNAR_SNITCH_minimal.vcxproj"
    
    result = subprocess.run(
        [msbuild_path, str(project_path), "/p:Configuration=Release", "/p:Platform=x64"],
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print(f"Plugin build failed: {result.stderr}")
        return False
    
    # Copy plugin to plugins directory
    plugin_src = Path(__file__).parent / "Source" / "LUNAR_SNITCH" / "Build" / "LUNAR_SNITCH_minimal.aex"
    plugin_dst = Path(__file__).parent / "plugins" / "LUNAR_FX.aex"
    plugin_dst.parent.mkdir(exist_ok=True)
    shutil.copy2(plugin_src, plugin_dst)
    
    print("Plugin built successfully!")
    return True

def build_installer():
    """Build the installer"""
    print("Building installer...")
    build_script = Path(__file__).parent / "Source" / "Installer" / "build_installer.py"
    
    result = subprocess.run([sys.executable, str(build_script)], capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"Installer build failed: {result.stderr}")
        return False
    
    print("Installer built successfully!")
    return True

def update_git():
    """Update git repository"""
    print("Updating git repository...")
    
    # Add all changes
    subprocess.run(["git", "add", "."], capture_output=True)
    
    # Commit changes
    import datetime
    version = datetime.datetime.now().strftime("%Y.%m.%d")
    commit_msg = f"Release {version}"
    subprocess.run(["git", "commit", "-m", commit_msg], capture_output=True)
    
    # Tag the release
    subprocess.run(["git", "tag", "-a", version, "-m", f"Release {version}"], capture_output=True)
    
    # Push to remote (if configured)
    # subprocess.run(["git", "push"], capture_output=True)
    # subprocess.run(["git", "push", "--tags"], capture_output=True)
    
    print("Git repository updated!")
    return True

def create_github_release():
    """Create GitHub release (requires GitHub CLI or API token)"""
    print("Creating GitHub release...")
    print("Note: This requires GitHub CLI (gh) to be installed and authenticated")
    print("Or you can manually create a release on GitHub with the Build folder contents")
    
    # This would require GitHub CLI or API token
    # For now, just print instructions
    print("\nManual steps for GitHub release:")
    print("1. Go to GitHub repository")
    print("2. Create a new release")
    print("3. Upload these files from the Build folder:")
    print("   - LUNAR_FX_Installer.exe")
    print("   - LUNAR_FX.aex (from plugins folder)")
    
    return True

def main():
    """Main release process"""
    print("LUNAR --FX Automated Release")
    print("=" * 40)
    
    if not build_plugin():
        print("Failed to build plugin")
        sys.exit(1)
    
    if not build_installer():
        print("Failed to build installer")
        sys.exit(1)
    
    if not update_git():
        print("Failed to update git")
        sys.exit(1)
    
    if not create_github_release():
        print("Failed to create GitHub release")
        sys.exit(1)
    
    print("\nRelease complete!")
    print("Files ready in Build folder:")
    print("  - LUNAR_FX_Installer.exe")
    print("  - plugins/LUNAR_FX.aex")

if __name__ == "__main__":
    main()
