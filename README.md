# GMake - GNUMake GUARDIAN Port

## Introduction

GMake is a GUARDIAN port of the GNUMake product. Technically, this is
a fork of the project, with the same file names and structure. The key
difference is that GMake operates in the GUARDIAN space instead of OSS although
contains much of the same capabilities of the 4.1 version.

## License

GMake is subject to the GPLv3 license. You make fork this repository
but are subject to all of the terms specified in the GNUMake license, meaning
you can do essentially what you want with the code, but you must make the
source available to anyone. The GPLv3 license can be found 
[here](https://www.gnu.org/licenses/gpl-3.0.en.html).

## Documentation

The main manual page for GNU Make can be found at
[https://www.gnu.org/software/make/manual/make.html](https://www.gnu.org/software/make/manual/make.html).
Please use that site as the definitive manual for using GMake except as
described in the extensions manual below. 

The [MANUAL.md](MANUAL.md) page describes the extensions and differences with
the GUARDIAN port, GMAKE.

## Problems and Enhancements

Use the GitHub issue tracker to report problems and request enhancements.

## Contributing

To contribute to GMake, you should fork the repository, make your changes,
and submit a pull request on GitHub. It is a good idea to set up an issue
describing the feature you are working on to give the ITUGLIB team a heads up
on your contribution.

You must fill out a [Contributor License Agreement](CONTRIBUTING.md)
and send it to one of the ITUGLIB team members so that we know you have the
authority to contribute your code.

## Technology

GMake is built using either NSDEE or OSS c99 compilers. GNUMake Makefiles are
supplied for J-series and L-series builds for OSS and NSDEE. GMake is
standalone and does not have dependencies to any other products. Standard
builds of GMake are available for TNS/E and TNS/X at
[ITUGLIB](https://ituglib.connect-community.org/apps/Ituglib/SrchOpenSrcLib.xhtml)
in PAK format.

| Makefile         | Use for                                |
| -----------------| -------------------------------------- |
| Makefile.NSE.OSS | Building GMake on OSS for J-series     |
| Makefile.NSE.Win | Building GMake with NSDEE for J-series |
| Makefile.NSX.OSS | Building GMake on OSS for L-series     |
| Makefile.NSX.Win | Building GMake with NSDEE for L-series |

You generally should specify the GMAKE_EXE make variable to specify the
destination of your build.

### Building GMake

To build GMake, use make, selecting the appropriate Makefile. For example
```
   make -f Makefile.NSX.OSS GMAKE_EXE=/G/data/subvol/gmake
```
**Note:** You must build GMake in OSS. Building in GUARDIAN is not supported
by this fork.

## Notes

`arscan.c` file uses ar.h header which is available only in cross compilers 
whereas the same is not available in GUARDIAN or oss host. Currently a copy 
of the ar.h header from cross compilers (J06.23/L19.08) is being used as 
`ar.nsx.h` and `ar.nse.h` in place of `ar.h`. To update these headers pick
up them from the latest cross compilers, assuming you have a license to those.
