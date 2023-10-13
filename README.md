## 依赖项

- R语言与lme4包

Build & Run

```bash
cd C:\WorkSpaceOnline\getPM2D5-old
set OSGEO4W_ROOT=C:\OSGeo4W
set GDAL_ROOT=C:\OSGeo4W
set OSGEO4W_QGIS_SUBDIR=qgis-ltr
set PATH=C:\Qt\5.15.2\msvc2019_64\bin;C:\OSGeo4W\bin\;C:\OSGeo4W\apps\Qt5\bin\;C:\OSGeo4W\apps\qgis-ltr\bin\;%PATH%

cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_MODULE_PATH="C:/OSGeo4W/apps/qgis-ltr" -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64"
cmake --build build-release --config Release

cp WSRHdata\ build-release\Release\
cp R\ build-release\Release\

cd  build-release\Release\
windeployqt getPM2D5.exe
getPM2D5.exe
```

