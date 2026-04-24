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
        ctk.set_appearance_mode("Dark")
        ctk.set_default_color_theme("blue")
        
        self.root = ctk.CTk()
        self.root.title("LUNAR --FX Installer")
        self.root.geometry("700x750")
        self.root.resizable(True, True)
        
        self.ae_versions = []
        self.selected_version = tk.StringVar()
        self.custom_path = tk.StringVar()
        self.install_path = tk.StringVar()
        
        if getattr(sys, 'frozen', False):
            self.installer_dir = Path(sys._MEIPASS)
            self.plugin_dir = self.installer_dir / "plugins"
        else:
            self.installer_dir = Path(__file__).parent.parent.parent
            self.plugin_dir = self.installer_dir / "plugins"
        
        self.setup_ui()
        self.detect_after_effects()
    
    def setup_ui(self):
        self.scroll_frame = ctk.CTkScrollableFrame(self.root)
        self.scroll_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
        
        header_frame = ctk.CTkFrame(self.scroll_frame)
        header_frame.pack(fill=tk.X, pady=(0, 20))
        
        title_label = ctk.CTkLabel(header_frame, text="LUNAR --FX", 
                                   font=ctk.CTkFont(size=32, weight="bold"))
        title_label.pack(anchor=tk.W, pady=(20, 5))
        
        subtitle_label = ctk.CTkLabel(header_frame, text="Let your edit shine like a moon", 
                                    font=ctk.CTkFont(size=14),
                                    text_color="gray")
        subtitle_label.pack(anchor=tk.W, pady=(0, 20))
        
        content_frame = ctk.CTkFrame(self.scroll_frame)
        content_frame.pack(fill=tk.X, pady=(0, 20))
        
        version_label = ctk.CTkLabel(content_frame, text="After Effects Version", 
                                    font=ctk.CTkFont(size=16, weight="bold"))
        version_label.pack(anchor=tk.W, pady=(10, 5))
        
        self.version_combo = ctk.CTkComboBox(content_frame, 
                                           values=[],
                                           command=self.on_version_selected)
        self.version_combo.pack(fill=tk.X, pady=(0, 10))
        
        self.custom_check = ctk.CTkCheckBox(content_frame, text="Use custom path", 
                                          command=self.toggle_custom_path)
        self.custom_check.pack(anchor=tk.W, pady=(10, 5))
        
        self.custom_path_entry = ctk.CTkEntry(content_frame, textvariable=self.custom_path)
        self.custom_path_entry.pack(fill=tk.X, pady=(5, 5))
        self.custom_path_entry.configure(state="disabled")
        
        self.browse_button = ctk.CTkButton(content_frame, text="Browse", 
                                          command=self.browse_path)
        self.browse_button.pack(anchor=tk.W, pady=(5, 10))
        self.browse_button.configure(state="disabled")
        
        path_label = ctk.CTkLabel(content_frame, text="Installation Path", 
                                  font=ctk.CTkFont(size=16, weight="bold"))
        path_label.pack(anchor=tk.W, pady=(10, 5))
        
        self.path_display = ctk.CTkLabel(content_frame, textvariable=self.install_path,
                                       text_color="gray")
        self.path_display.pack(anchor=tk.W, pady=(0, 10))
        
        plugin_label = ctk.CTkLabel(content_frame, text="Detected Plugins", 
                                   font=ctk.CTkFont(size=16, weight="bold"))
        plugin_label.pack(anchor=tk.W, pady=(10, 5))
        
        self.plugin_text = ctk.CTkTextbox(content_frame, height=100)
        self.plugin_text.pack(fill=tk.X, pady=(0, 20))
        
        install_button = ctk.CTkButton(content_frame, text="Install LUNAR --FX", 
                                     command=self.install_plugins,
                                     height=50)
        install_button.pack(fill=tk.X, pady=(0, 20))
        
        self.custom_path_entry.configure(state="disabled")
        self.browse_button.configure(state="disabled")
        
        self.check_plugins()
    
    def detect_after_effects(self):
        self.ae_versions = []
        
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
                            if version and self.is_valid_version(version):
                                self.ae_versions.append({
                                    'name': item,
                                    'version': version,
                                    'path': str(ae_path)
                                })
            except (OSError, PermissionError):
                continue
        
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
                            if self.is_valid_version(version):
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
        try:
            version_float = float(version)
            if version_float < 100:
                version_year = int(version_float) + 2003
            else:
                version_year = int(version_float)
            return version_year >= 2020
        except (ValueError, TypeError):
            return False
    
    def extract_version_from_name(self, name):
        import re
        match = re.search(r'(\d{4})', name)
        return match.group(1) if match else None
    
    def on_version_selected(self, event):
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
        if self.custom_check.get():
            self.custom_path_entry.configure(state="normal")
            self.browse_button.configure(state="normal")
            self.version_combo.configure(state="disabled")
            self.update_install_path()
        else:
            self.custom_path_entry.configure(state="disabled")
            self.browse_button.configure(state="disabled")
            self.version_combo.configure(state="normal")
            if self.ae_versions:
                self.on_version_selected(None)
    
    def browse_path(self):
        path = filedialog.askdirectory(title="Select Installation Directory")
        if path:
            self.custom_path.set(path)
            self.update_install_path()
    
    def update_install_path(self):
        if self.custom_check.get():
            lunar_path = Path(self.custom_path.get()) / "LUNAR --FX"
            self.install_path.set(str(lunar_path))
    
    def install_plugins(self):
        install_path = Path(self.install_path.get())
        
        if not install_path.parent.exists():
            messagebox.showerror("Error", "Parent directory does not exist")
            return
        
        plugin_files = list(self.plugin_dir.glob("*.aex"))
        if not plugin_files:
            messagebox.showerror("Error", 
                                "No plugin files found in Build directory.\n"
                                "Please build the plugins first.")
            return
        
        try:
            install_path.mkdir(parents=True, exist_ok=True)
            
            for plugin_file in plugin_files:
                shutil.copy2(plugin_file, install_path / plugin_file.name)
            
            messagebox.showinfo("Success", "LUNAR --FX installed successfully!")
        except Exception as e:
            messagebox.showerror("Error", f"Installation failed: {str(e)}")
    
    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    installer = LUNARInstaller()
    installer.run()
