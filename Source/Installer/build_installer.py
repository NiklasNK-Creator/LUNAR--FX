#!/usr/bin/env python3
"""
Build script for LUNAR --FX Installer
Creates a standalone executable from the installer script
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

def install_pyinstaller():
    """Install PyInstaller if not already installed"""
    try:
        import PyInstaller
        print("PyInstaller is already installed")
        return True
    except ImportError:
        print("Installing PyInstaller...")
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", "pyinstaller"])
            return True
        except subprocess.CalledProcessError as e:
            print(f"Failed to install PyInstaller: {e}")
            return False

def build_installer():
    """Build the standalone installer executable"""
    installer_script = Path(__file__).parent / "installer.py"
    output_dir = Path(__file__).parent.parent.parent / "Build"
    plugin_dir = Path(__file__).parent.parent.parent / "plugins" / "LUNAR_FX.aex"
    readme_path = Path(__file__).parent.parent.parent / "README.md"
    manifest_path = Path(__file__).parent / "app.manifest"
    
    # Ensure output directory exists
    output_dir.mkdir(exist_ok=True)
    
    # PyInstaller command
    add_data_option = f"--add-data={plugin_dir};plugins"
    add_readme_option = f"--add-data={readme_path};."
    cmd = [
        sys.executable, "-m", "PyInstaller",
        "--onefile",                    # Create single executable
        "--windowed",                   # Hide console window (GUI app)
        "--uac-admin",                 # Request admin rights
        "--name=LUNAR_FX_Installer",    # Output executable name
        f"--distpath={output_dir}",     # Output directory
        f"--workpath={os.path.join(output_dir, 'build')}", # Build directory
        f"--specpath={output_dir}",     # Spec file location
        "--hidden-import=customtkinter",
        "--hidden-import=darkdetect",
        "--hidden-import=packaging",
        add_data_option,
        add_readme_option,
        str(installer_script)
    ]
    
    print("Building LUNAR --FX Installer...")
    print(f"Command: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        print("Build completed successfully!")
        print(f"Installer created: {output_dir / 'LUNAR_FX_Installer.exe'}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Build failed: {e}")
        print(f"Stdout: {e.stdout}")
        print(f"Stderr: {e.stderr}")
        return False
    except FileNotFoundError:
        print("PyInstaller not found. Please install it first.")
        return False

def create_requirements():
    """Create requirements.txt file"""
    requirements = [
        "pyinstaller>=6.0.0",
        "customtkinter>=5.0.0"
    ]
    
    req_file = Path(__file__).parent / "requirements.txt"
    with open(req_file, 'w') as f:
        f.write('\n'.join(requirements))
    
    print(f"Created requirements.txt: {req_file}")

def main():
    """Main build process"""
    print("LUNAR --FX Installer Build Script")
    print("=" * 40)
    
    # Create requirements file
    create_requirements()
    
    # Install PyInstaller
    if not install_pyinstaller():
        print("Failed to install PyInstaller. Aborting build.")
        return False
    
    # Build the installer
    if build_installer():
        print("\nBuild completed successfully!")
        print("You can now run the installer from the Build directory.")
        return True
    else:
        print("\nBuild failed. Check the error messages above.")
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
