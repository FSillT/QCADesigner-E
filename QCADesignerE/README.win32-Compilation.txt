QCADesignerE: Windows Compilation Instructions:


* install mingw (yum install mingw*) => 32 and 64bit version
* download GTK2 - windows bundle: https://sourceforge.net/projects/gtk-mingw/files/gtk%2B2/ ( gtk+-2.24.11-bundle.7z)
* extract to /opt/gtkwin
* Type
  * cd /opt/gtkwin
  * find -name '*.pc' | while read pc; do sed -e "s@^prefix=.*@prefix=$PWD@" -i "$pc"; done
  * sudo ln -s /opt/gtkwin/include/pango-1.0/ usr/i686-w64-mingw32/sys-root/mingw/include/.
  * sudo ln -s /opt/gtkwin/include/cairo usr/i686-w64-mingw32/sys-root/mingw/include/.
* (recommended: create temporary copy of QCADesigner)
* enter QCADesigner_copy path and type
  * chmod 755 autogen.sh
  * chmod 755 configure
  * ./autogen.sh
  * export PKG_CONFIG_PATH=/opt/gtkwin/lib/pkgconfig
  * ./configure --host=i686-w64-mingw32 --build=x86_64-w64-mingw32 --prefix=/home/frank/QCA/QCAEnergy_win/bin
  * sed -e 's/-lintl//g' -i Makefile
  * sed -e 's/-lintl//g' -i src/Makefile
  * rm src/*.o src/objects/*.o
  * make
  * make install
* Use the src/QCADesigner.exe
* Note: all *.o files have to be removed (from linux compilation)
* Note: reset PKG_CONFIG_PATH (export PKG_CONFIG_PATH=) if you compile for Linux agian


Inno Setup:
* copy QCADesignerE-win32.iss to folder 'windows/ISS' 
* copy bin and share folder to 'windows/QCADesignerE' 
* copy windows folder to WINDOWS and execute InnoSetup
* exchange $ADD_INNOHOME$ in QCADesignerE-win32.iss with actual windows folder name (that contains the folder bin and share)
