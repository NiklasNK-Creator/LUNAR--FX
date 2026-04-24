#!/usr/bin/env python3
"""
LUNAR --FX Installer
Installs LUNAR.SNITCH and other LUNAR --FX plugins to Adobe After Effects
Supports After Effects 2020 and newer versions
"""

import os
import sys
import shutil
import json
import winreg
from pathlib import Path
from typing import List, Dict, Optional
import tkinter as tk
import customtkinter as ctk
from tkinter import messagebox, filedialog
import subprocess

class LUNARInstaller:
    def __init__(self):
        # Set appearance mode
        ctk.set_appearance_mode("Dark")
        ctk.set_default_color_theme("blue")
        
        self.root = ctk.CTk()
        self.root.title("LUNAR --FX Installer")
        self.root.geometry("700x750")
        self.root.resizable(True, True)
        
        # Variables
        self.ae_versions = []
        self.selected_version = tk.StringVar()
        self.custom_path = tk.StringVar()
        self.install_path = tk.StringVar()
        
        # Get current directory where installer is running
        # Try bundled data first, then fallback to local directory
        if getattr(sys, 'frozen', False):
            # Running as bundled exe
            self.installer_dir = Path(sys._MEIPASS)
            self.plugin_dir = self.installer_dir / "plugins"
        else:
            # Running as script
            self.installer_dir = Path(__file__).parent.parent.parent
            self.plugin_dir = self.installer_dir / "plugins"
        
        self.setup_ui()
        self.detect_after_effects()
    
    def setup_ui(self):
        # Main scrollable frame
        self.scroll_frame = ctk.CTkScrollableFrame(self.root)
        self.scroll_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
        
        # Header section
        header_frame = ctk.CTkFrame(self.scroll_frame)
        header_frame.pack(fill=tk.X, pady=(0, 20))
        
        # Title
        title_label = ctk.CTkLabel(header_frame, text="LUNAR --FX", 
                                   font=ctk.CTkFont(size=32, weight="bold"))
        title_label.pack(anchor=tk.W, pady=(20, 5))
        
        # Subtitle
        subtitle_label = ctk.CTkLabel(header_frame, text="Let your edit shine like a moon", 
                                    font=ctk.CTkFont(size=14),
                                    text_color="gray")
        subtitle_label.pack(anchor=tk.W, pady=(0, 20))
        
        # Content frame
        content_frame = ctk.CTkFrame(self.scroll_frame)
        content_frame.pack(fill=tk.X, pady=(0, 20))
        
        # Version selection section
        version_label = ctk.CTkLabel(content_frame, text="After Effects Version", 
                                    font=ctk.CTkFont(size=12, weight="bold"))
        version_label.pack(anchor=tk.W, pady=(20, 10))
        
        self.version_combo = ctk.CTkComboBox(content_frame, 
                                          values=["Detecting..."],
                                          variable=self.selected_version,
                                          state="readonly")
        self.version_combo.pack(fill=tk.X, pady=(0, 20))
        self.version_combo.bind('<<ComboboxSelected>>', self.on_version_selected)
        
        # Custom path option
        self.custom_check = ctk.CTkCheckBox(content_frame, text="Use custom installation path",
                                           command=self.toggle_custom_path)
        self.custom_check.pack(anchor=tk.W, pady=(10, 10))
        
        # Custom path entry
        self.custom_path_entry = ctk.CTkEntry(content_frame, 
                                            placeholder_text="Custom installation path",
                                            textvariable=self.custom_path)
        self.custom_path_entry.pack(fill=tk.X, pady=(0, 10))
        
        self.browse_button = ctk.CTkButton(content_frame, text="Browse", 
                                         command=self.browse_path,
                                         width=100)
        self.browse_button.pack(anchor=tk.E, pady=(0, 20))
        
        # Installation path display
        path_display_label = ctk.CTkLabel(content_frame, text="Installation Path", 
                                        font=ctk.CTkFont(size=12, weight="bold"))
        path_display_label.pack(anchor=tk.W, pady=(20, 10))
        
        self.path_label = ctk.CTkLabel(content_frame, textvariable=self.install_path,
                                     text_color="#58a6ff",
                                     wraplength=600)
        self.path_label.pack(anchor=tk.W, pady=(0, 20))
        
        # Plugins section
        plugins_label = ctk.CTkLabel(content_frame, text="Bundled Plugins", 
                                    font=ctk.CTkFont(size=12, weight="bold"))
        plugins_label.pack(anchor=tk.W, pady=(0, 10))
        
        self.plugin_text = ctk.CTkTextbox(content_frame, height=100)
        self.plugin_text.pack(fill=tk.X, pady=(0, 20))
        
        # Install button
        self.install_button = ctk.CTkButton(content_frame, text="Install LUNAR --FX", 
                                          command=self.install_plugins,
                                          height=40,
                                          font=ctk.CTkFont(size=14, weight="bold"))
        self.install_button.pack(fill=tk.X, pady=(20, 20))
        
        # Initial state
        self.custom_path_entry.configure(state="disabled")
        self.browse_button.configure(state="disabled")
        
        # Check for plugins
        self.check_plugins()
    
    def detect_after_effects(self):
        """Detect installed After Effects versions"""
        self.ae_versions = []
        
        # Check common Adobe installation paths
        adobe_paths = [
            r"C:\Program Files\Adobe",
            r"C:\Program Files (x86)\Adobe"
        ]
        
        for adobe_path in adobe_paths:
            if not os.path.exists(adobe_path):
                continue
            
            try:
                for item in os.listdir(adobe_path):
                    if item.startswith("Adobe After Effects"):
                        ae_path = Path(adobe_path) / item
                        if ae_path.is_dir():
                            version = self.extract_version_from_name(item)
                            if version and self.is_valid_version(version):  # Only 2020+
                                self.ae_versions.append({
                                    'name': item,
                                    'version': version,
                                    'path': str(ae_path)
                                })
            except (OSError, PermissionError):
                continue
        
        # Also check registry for additional installations
        try:
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, 
                              r"SOFTWARE\Adobe\After Effects") as key:
                i = 0
                while True:
                    try:
                        subkey_name = winreg.EnumKey(key, i)
                        try:
                            version_path = winreg.OpenKey(key, subkey_name)
                            install_path = winreg.QueryValueEx(version_path, "InstallPath")[0]
                            
                            version = subkey_name
                            if self.is_valid_version(version):  # Only 2020+
                                self.ae_versions.append({
                                    'name': f"Adobe After Effects {subkey_name}",
                                    'version': subkey_name,
                                    'path': install_path
                                })
                        except (OSError, FileNotFoundError):
                            pass
                        i += 1
                    except (OSError, FileNotFoundError):
                        break
        except (OSError, FileNotFoundError):
            pass
        
        # Update combo box
        if self.ae_versions:
            version_names = [f"{ae['name']} (v{ae['version']})" for ae in self.ae_versions]
            self.version_combo.configure(values=version_names)
            if version_names:
                self.version_combo.set(version_names[0])
                self.on_version_selected(None)
        else:
            self.version_combo.configure(values=["No After Effects 2020+ found"])
            self.version_combo.set("No After Effects 2020+ found")
    
    def is_valid_version(self, version):
        """Check if version is 2020 or newer"""
        try:
            # Handle both integer and float versions (e.g., "2020" or "17.0")
            version_float = float(version)
            # If version is like "17.0", convert to year (17.0 = 2020)
            if version_float < 100:
                version_year = int(version_float) + 2003  # AE 1.0 = 2003, so 17.0 = 2020
            else:
                version_year = int(version_float)
            return version_year >= 2020
        except (ValueError, TypeError):
            return False
    
    def extract_version_from_name(self, name):
        """Extract version number from folder name"""
        import re
        match = re.search(r'(\d{4})', name)
        return match.group(1) if match else None
    
    def on_version_selected(self, event):
        """Handle version selection"""
        if not self.ae_versions:
            return
        
        selected = self.version_combo.get()
        for ae_info in self.ae_versions:
            if f"{ae_info['name']} (v{ae_info['version']})" == selected:
                plugins_path = Path(ae_info['path']) / "Support Files" / "Plug-ins"
                lunar_path = plugins_path / "LUNAR --FX"
                self.install_path.set(str(lunar_path))
                break
    
    def toggle_custom_path(self):
        """Toggle custom path entry"""
        if self.custom_check.get():
            self.custom_path_entry.configure(state="normal")
            self.browse_button.configure(state="normal")
            self.version_combo.configure(state="disabled")
            self.update_install_path()
        else:
            self.custom_path_entry.configure(state="disabled")
            self.browse_button.configure(state="disabled")
            self.version_combo.configure(state="readonly")
            self.on_version_selected(None)
    
    def browse_path(self):
        """Browse for custom installation path"""
        path = filedialog.askdirectory(title="Select After Effects Plug-ins Directory")
        if path:
            self.custom_path.set(path)
            self.update_install_path()
    
    def update_install_path(self):
        """Update installation path display"""
        if self.custom_path.get():
            lunar_path = Path(self.custom_path.get()) / "LUNAR --FX"
            self.install_path.set(str(lunar_path))
    
    def check_plugins(self):
        """Check for available plugins in plugins directory"""
        plugins = []
        
        if self.plugin_dir.exists():
            for file_path in self.plugin_dir.glob("*.aex"):
                plugins.append(f"✓ {file_path.name}")
        
        if plugins:
            plugin_text = "\n".join(plugins)
        else:
            plugin_text = "No plugin files found.\nPlugins should be bundled with the installer."
        
        self.plugin_text.delete("0.0", "end")
        self.plugin_text.insert("0.0", plugin_text)
    
    def install_plugins(self):
        """Install plugins to selected directory"""
        install_path = Path(self.install_path.get())
        
        if not install_path.parent.exists():
            messagebox.showerror("Error", 
                                f"Parent directory does not exist:\n{install_path.parent}")
            return
        
        # Check if plugins exist
        plugin_files = list(self.plugin_dir.glob("*.aex"))
        if not plugin_files:
            messagebox.showerror("Error", 
                                "No plugin files found in Build directory.\n"
                                "Please build the plugins first.")
            return
        
        try:
            # Create LUNAR --FX directory
            install_path.mkdir(parents=True, exist_ok=True)
            
            # Copy plugin files
            copied_files = []
            for plugin_file in plugin_files:
                dest_path = install_path / plugin_file.name
                shutil.copy2(plugin_file, dest_path)
                copied_files.append(plugin_file.name)
            
            # Create info file
            info_file = install_path / "LUNAR_FX_Info.txt"
            with open(info_file, 'w') as f:
                f.write("LUNAR --FX Plugin Collection\n")
                f.write(f"Installed: {len(copied_files)} plugins\n")
                f.write("Plugins:\n")
                for plugin in copied_files:
                    f.write(f"  - {plugin}\n")
            
            messagebox.showinfo("Installation Complete", 
                              f"Successfully installed {len(copied_files)} LUNAR --FX plugins to:\n"
                              f"{install_path}\n\n"
                              f"Plugins:\n" + "\n".join(f"  - {p}" for p in copied_files) +
                              f"\n\nRestart After Effects to see the new effects.")
            
        except Exception as e:
            messagebox.showerror("Installation Failed", 
                               f"Failed to install plugins:\n{str(e)}")
    
    def run(self):
        """Start the installer GUI"""
        self.root.mainloop()

def main():
    """Main entry point"""
    try:
        installer = LUNARInstaller()
        installer.run()
    except Exception as e:
        messagebox.showerror("Installer Error", f"Failed to start installer:\n{str(e)}")

if __name__ == "__main__":
    main()
