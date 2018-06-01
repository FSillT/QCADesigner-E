Windows Installation Instructions:

1. Go to http://dropline.net/gtk and download "GTK+ Runtime Environment"

2. Install it, and remember the installation path

3. You must now add the \lib subdirectory of this "GTK+ Runtime Environment"
   to your PATH:
   In Windows 95/98 (and possibly ME), edit C:\AUTOEXEC.BAT and append the
   installation path from step 2 to the end of the file using the following 
   line:
   
   PATH=%PATH%;<installation_path>\lib
   
   Above, replace <installation_path> with the path from step 2

   On Windows 2000/XP, you must go to
   Control Panel -> System -> Advanced -> Environment Variables
   There, edit the one called either PATH or Path and append
   
   ;<installation_path>\lib
   
   to it.  Click "OK" all the way back to Control Panel.  Above,
   <installation_path> refers to the path from step 2.

4. Run QCADesigner !

The rest of the steps are required only if you would like to produce PostScript
files of your design using QCADesigner's printing facilities:

5. Grab GhostView and Ghostscript from http://www.cs.wisc.edu/~ghost/

6. Install them using default settings.

7. Run QCADesigner, open or create a design and click "Print Preview".

8. QCADesigner will now prompt you to browse for a PostScript viewer.  Ghostview is
   such a viewer.  Browse over to where you installed Ghostview (usually, this is
   C:\Program Files\Ghostgum\gsview) and select gsview32.exe.

9. Behold, your design rendered as beautiful PostScript !

Since the Windows version of QCADesigner can not yet print on its own, you should
really consider using Ghostview as your "Print Preview" application.  That way, you 
can print your designs from there.
