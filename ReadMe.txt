TODO:
If there is selection, cursor commands should move to the selection boundary
check if whitespace difference is attached to more spaces. Try to rearrange
    segments to avoid misreading. Glue the whitespace to more space.
update main frame during long comparision. Process Esc key. Pump commands.

show version of file1, file2 of the line
Show Left-only, right-only, merged line (single line only)
make dir names and full file names available either in properties sheet or in whatever
Add Save differences
Add save file list
Make Go To Line to first or second file line number
Import font settings from Visual C/VisualStudio
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Make progress dialog for binary comparision
Make background thread for comparision
Cancel selection on Accept/Decline commands (to see the result immediately). ???
Maybe different color for selected accepted/declined
UNICODE file support
//show subdir-only names in the status bar during comparision
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:

Fixed:
If the file is already open, it won't be activated from the list view
horizontal autoscroll is wrong (one character not updated)
National characters are not recognized as letters (prevent sign expansion!)
forward search next can't continue sometimes if there are whitespaces
can't find next diff with whitespaces in the line
goto next diff doesn't work sometimes when whitespaces are on
fast rolling of mouse wheel confuses scrolling
page down in short file causes crash
Word Right doesn't skip spaces after the word
Accept/Decline for a pair doesn't change the second
Line numbers were zero-bazed
Auto Scroll is too fast
Use Collate to sort the filenames
Tries to reload files during modal state

Done:

Support Ctrl+Up, Ctrl+Down
Caret movements now go from the selection boundaries
save Show Toolbar, Show Status Bar options
Make search dialog
Add Ctrl+F6 interception for window switch
Add progress to File list view and file diff view
Ctrl+mouse selection - make full words
Change concept to include/exclude, use file 1, use file2
Context menu on selection: Merge File1 version, Merge File2 version
mark accept/decline line by line
make selective accept/decline, for some lines of group
Remember last sort in list view
Make copy to folder for the open file
Open new frame with the same size attr as active frame
