TODO:
show version of file1, file2 of the line
Show Left-only, right-only, merged line (single line only)
Show line number from first and second file
Make Go To Line to first or second file line number
Import font settings from Visual C/VisualStudio
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Make progress dialog for binary comparision
Cancel selection on Accept/Decline commands (to see the result immediately). ???
Maybe different color for selected accepted/declined
on F7, make the whole diff visible
add option to keep the caret on one line during F3 and F7
UNICODE file support
Add Save differences
Add save file list
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:
When there are much different areas, they may be randomly interlaced.
   Solution: if there is little common, just ignore it.
screen move in two directions may corrupt the image (??)
Ctrl-mouse selection doesn't retain proper anchor position

Fixed:
Comparison Preferences dialog labels are too short
Word on the end of the line can't be found
memory is not freed in time (files not unloaded) after the file was refreshed and closed
doesn't show removed lines at the end
inserts lots of empty lines on merge
merge doesn't work

Done:
Drag and drop support in the compare files dialog
Put comparision result to the status string after the file is open
Make file comparison dialog, similar to dir comparison
Close About dialog on MailTo button
mailto: button in About dialog
make dir names and full file names available either in properties sheet or in whatever
Show file properties
show progress while comparing the files in the thread
make different secondary sort after comparison result sort
Make background thread for comparision
update main frame during long comparision. Process Esc key. Pump commands.

check if whitespace difference is attached to more spaces. Try to rearrange
    segments to avoid misreading. Glue the whitespace to more space.
Support Ctrl+Up, Ctrl+Down
Caret movements now go from the selection boundaries
save Show Toolbar, Show Status Bar options
Make search dialog
Add Ctrl+F6 interception for window switch
Add progress to File list view and file diff view
