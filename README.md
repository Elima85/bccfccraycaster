# BccFccRaycaster

## Introduction
BccFccRaycaster is a tool for visualizing data sampled on body-centered cubic (BCC) and face-centered cubic (FCC) lattices. It is implemented as a patch for the volume renderer [Voreen 3.0.1](voreen.org), a copy of which is included in this repository.

## Features
 * BCCVolumeRaycaster
    * nearest neighbor interpolation (z-interleaved storage recommended!)
    * linear box-spline interpolation (z-interleaved storage recommended!) [Finkbeiner et al., 2009](http://onlinelibrary.wiley.com/doi/10.1111/j.1467-8659.2009.01445.x/abstract?userIsAuthenticated=false&deniedAccessCustomisedMessage=), [Finkbeiner et al., 2010](http://www.sciencedirect.com/science/article/pii/S0097849310000245)
    * DC-spline interpolation [Domonkos and Csébfalvi, 2010](http://sirkan.iit.bme.hu/~domi/publications/index.php?pub=2010-vmv)
    * cosine-weighted B-spline interpolation [Csébfalvi, 2013](http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=6409843&url=http%3A%2F%2Fieeexplore.ieee.org%2Fxpls%2Fabs_all.jsp%3Farnumber%3D6409843)
 * FCCVolumeRaycaster
    * nearest neighbor interpolation
    * DC-spline interpolation [Domonkos and Csébfalvi, 2010](http://sirkan.iit.bme.hu/~domi/publications/index.php?pub=2010-vmv)

## Building
The building procedure is identical to that of [Voreen 3.0.1](http://voreen.org/223-0-Getting-Started.html).

### Requirements
* Qt 4.5
* GLEW 1.5
* DevIL

### Build instructions

#### Windows
Rename **config-default.txt** to **config.txt**.
##### With Qt Visual Studio Addon
In Visual Studio, go to **Qt** -> **Open Solution from .pro File**, and select **voreen.pro**.
##### Without Qt Visual Studio Addon
Please see the instructions [here](http://voreen.org/99-Build-Instructions.html).

#### Linux
In the root directory, call
```bash
cp config-default.txt config.txt
qmake voreen.pro
make
```

## Generating test volumes
Test volumes can be produced using [mkvol](https://github.com/Elima85/mkvol).

## Voreen 3.0.1
This information is copied from readme.txt, obtained from voreen.org.

### The Voreen directory structure:
 * include: contains all necessary headers
 * src: contains the source files
 * modules: contains all files for plugin modules
 * apps: contains applications using voreen
 * apps/voreenve: the main voreen application - the Voreen Visualization Environment
 * apps/voltool: perform various preprocessings on volume data
 * ext: external dependencies to other libraries
 * data: contains volume datasets
 * data/cache: a temporary folder used for volume caching
 * data/fonts: (possibly) necessary fonts
 * data/networks: predefined networks which can be used in VoreenVE
 * data/scripts: python scripts which can be executed from within voreen
 * data/workspaces: predefined workspace to be used with VoreenVE
 * data/volume: default directory for volume data
 * data/transferfuncs: contains transfer functions in various formats
