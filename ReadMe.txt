TODO:
make word by word comparison in MatchStrings
Refresh list view
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
Marking single line difference doesn't seem to call SetModified
Concatenate the file diff sections of the same type (whole line)
Empty lines on the end of the difference area are shown as different.
breaks on sort by date modified
Text Discarded From Merge label doesn't fit
match whole word only label doesn't fit
File name in Properties is not wrapped
Compare files as C/C++ label doesn't fit and other checkbox labels too
make View Differences in the main menu non-bold

Done:
Select the current folder in Recent Dirs in open dialog
When merging two files, get the default name from the base file
Uncheck "Compare files as binary" as default
eradicate phantom directory column, when subdirs are disabled
Empty lines shown as one-space lines
when comparing the files during open, update comparison result
Put dropped files to history
