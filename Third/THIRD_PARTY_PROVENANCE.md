# Third-Party Source Provenance

The files below are vendored in this fork because the Exodus Visual Studio
projects require them and upstream intentionally ignores the extracted source
directories. They are build inputs, not Exodus product code.

| Dependency | Version | Original download URL | Required directory | License file |
| --- | --- | --- | --- | --- |
| zlib | 1.2.8 | `http://www.zlib.net/zlib128.zip` | `Third/zlib/zlib-1.2.8/` | `README` |
| libjpeg | 9a | `http://ijg.org/files/jpegsr9a.zip` | `Third/libjpeg/jpeg-9a/` | `README` |
| libpng | 1.6.12 | `http://download.sourceforge.net/libpng/lpng1612.zip` | `Third/libpng/lpng1612/` | `LICENSE` |
| libtiff | 4.0.9 | `http://download.osgeo.org/libtiff/tiff-4.0.9.zip` | `Third/libtiff/tiff-4.0.9/` | `README` |
| expat | 2.1.0 | `http://sourceforge.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz` | `Third/expat/expat-2.1.0/` | `COPYING` |

The original archive checksums were not retained with the historical local
source snapshot from which these files were recovered. Before replacing or
updating any dependency, record the downloaded archive SHA-256 here and verify
its extraction layout before committing it.

The source directories were recovered on 2026-08-22 from a local Exodus 2.1
build tree that had previously compiled successfully. Keep this document and
the dependency license notices in all source redistributions.
