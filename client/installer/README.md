# Packaging Oil Client

## Bootstrap

* Create directories `tools` and `out`
* Download latest xulrunner into `tools/xulrunner` from [http://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/latest/runtimes]
* Copy `setenv.bat.tmpl` as `tools/setenv.bat`, and fill with right paths to WiX and 7zip.
* Add `oil.ico` into `tools/xulrunner/xulrunner-stub.exe` with ResourceHacker.

## Making new version

* put right version into `version.txt`
* run `build.bat`. WARNING: result .msi will be created in `C:\tmp\`! That's because of strange glitches when it's generating in `out` directory.
