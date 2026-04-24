# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['D:\\projekte\\LUNAR --FX\\Source\\Installer\\installer.py'],
    pathex=[],
    binaries=[],
    datas=[('D:\\projekte\\LUNAR --FX\\Build\\LUNAR_SNITCH.aex', 'plugins'), ('D:\\projekte\\LUNAR --FX\\Build\\LUNAR_IMP.aex', 'plugins'), ('D:\\projekte\\LUNAR --FX\\README.md', '.')],
    hiddenimports=['customtkinter', 'darkdetect', 'packaging'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='LUNAR_FX_Installer',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    uac_admin=True,
)
