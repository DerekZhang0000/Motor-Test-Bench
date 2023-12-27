# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

a = Analysis(['test_bench_UI.py'],
             pathex=['test_bench_UI.py'],
             binaries=[],
             datas=[('assets', 'assets')],
             hiddenimports=[],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher,
             noarchive=False)
pyz = PYZ(a.pure, a.zipped_data,
           cipher=block_cipher)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          [],
          name='JAD Test Bench',
          debug=False,
          bootloader_ignore_signals=False,
          bootloader_silent=False,
          upx=True,
          upx_exclude=[],
          runtime_tmpdir=None,
          icon='assets/app-logo.ico',
          onefile=True,
          console=True)
