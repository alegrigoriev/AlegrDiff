TODO:
Add Line Numbers at the left margin
Make Word Left, Word Right, Word selection.
Make Go To Line
Make search
Add context menu to list view
Add context menu to file view
Make background thread for comparision
Add progress to File list view and file diff view
Check file dates for change, when the app is activated
Add "Refresh" function
Add Save differences
Add save file list
Add Copy files
Open directories or files from the command line
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Make progress dialog for binary comparision
UNICODE file support

Problems:

Fixed:
If two files are completely different, only first file is shown
BIG file takes too much to read. Eliminated array reallocation.
File with differences: last lines not drawn
Wrong status for inserted and removed parts of strings

Done:
Added buffering to file read
Remember Used file filter 
Add font selection
Expand tabs during file loading
Make "Copy to clipboard"
Make "Home", "End" functions
Make "Move to next diff", Move to prev diff functions
Draw selection in the file view
Use standard background for drawing text
Add Options dialog
File pattern support in the dialog.
Allow two-file comparision
Open list view only after directory names are entered
Show correct title for list view
Show title for file view
Add full comparision result in list view and sorting
Allow multiple views for file pair (reference count ??)
OnWindowView must duplicate file view only
Process Enter key in list view
File view with line-wize differences
File view with in-line differences
