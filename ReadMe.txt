TODO:
Save File list
Refresh list view
make word by word comparison in MatchStrings
show version of file1, file2 of the line
Show Left-only, right-only, merged line (single line only)
Import font settings from Visual C/VisualStudio
Cancel selection on Accept/Decline commands (to see the result immediately). ???
Maybe different color for selected accepted/declined
add option to keep the caret on one line during F3 and F7
V1.5:
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Show file version (add handler code)
Make progress dialog for binary comparision
UNICODE file support
Add Save differences
Add save file list
// for text editor:
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:
screen move in two directions may corrupt the image (??)

Fixed:
Single file opened, loses its comparision result in the list
If two completely different files, second is shorter, it would split the first file
When there are much different areas, they may be randomly interlaced.
   Solution: if there is little common, just ignore it.

Done:
drop directories to folder compare dialog
Select the current folder in Recent Dirs in open dialog
When merging two files, get the default name from the base file
Uncheck "Compare files as binary" as default
eradicate phantom directory column, when subdirs are disabled
Empty lines shown as one-space lines
when comparing the files during open, update comparison result
Put dropped files to history
