TODO:
Context menu on selection: Merge File1 version, Merge File2 version
Make copy to folder for the open file
mark accept/decline line by line
make selective accept/decline, for some lines of group
show version of file1, file2 of the line
Show Left-only, right-only, merged line (single line only)
Change concept to include/exclude, use file 1, use file2
Cancel selection on Accept/Decline commands (to see the result immediately). ???
Maybe different color for selected accepted/declined
Add Save differences
Add save file list
Make search dialog
Make Go To Line to first or second file line number
Import font settings from Visual C/VisualStudio
Add progress to File list view and file diff view
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Make progress dialog for binary comparision
Make background thread for comparision
UNICODE file support
//show subdir-only names in the status bar during comparision

Problems:
Tries to reload files during modal state
Auto Scroll is too fast

Fixed:
crashed on some files
F7 doesn't adjust for whitespaces
Blank lines are ignored during comparison, but may not be inored when line pair list built
Wrong button order in View properties
Inserted line with only spaces is not whitespace
Child window is created not maximized
Esc closes without prompting to save merged
Go To next differences when whitespaces ignored

Done:
Open new frame with the same size attr as active frame
Catch Ctrl+Tab, neat processing
last line is not invalidated by InvalidateRange
Separate menus for list view and diff view
swap file1 and file2 only in view preferences
Open directories or files from the command line
Processed blank lines during section expansion
make separate selection for background
make "Number Of matching chars" in preferences
Make combobox for last file filters
in InvalidateRange, add for letter overlap.
Make Word Left, Word Right.
Show added whitespaces
