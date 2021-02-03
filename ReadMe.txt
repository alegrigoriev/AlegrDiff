TODO:

Copy binary dump as text (if cursor on hex)
Add option for file size format (in kilobytes or in bytes) or a tooltip.
Customizable number of MRU items
Make more robust argorithm of version info detection (if it is version line, don't include version data to a normalized CRC).
Detect Log: (as comment sequence)

2. Add a pop-up menu item to rescan the differences while in file comparison mode. 

Allow doublequotes in the search pattern
Use OnActivateApp to refresh the files
Make sure to report read errors and file unaccessible errors during fingerprint creation.
make word by word comparison in MatchStrings
Make HTML help
Show properties for selected files (total size)

Maybe different color for selected accepted/declined
add option to keep the caret on one line during F3 and F7

V1.5:
Need a hot-key to select current changed block
Detect moved text blocks
Compare 3 files for merge
//Consider "signed fingerprint file" (UNICODE only) (can use secondary signature)
Compare EXE version info for DLL, EXE, SYS
Show file version (add handler code)
Add Save differences
process linux-style MD5 signature files
// for text editor:
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:
OnViewRefresh not defined for binary files
"Parent directory exists only in" should only print the name of base directory (or )
Refresh command disabled for fingerprint check
Directory share name cannot be selected if no '\' appended
If the text comparison got aborted because of read error, this is not shown
Blank line immediately following difference block is considered separate 
   for "Next Difference" command
Blank lines prevent from proper line matching inside difference blocks
Directory dialog allows to select network host. It won't return anything then.
Estimated time left is not shown while calculating a fingerpring of a big file  (???)

Retracted:
screen move in two directions may corrupt the image (??)

New state codes:
Reading first file
Reading second file


For text output:
1. Call GetTextExtentExPoint to get text extent array.
2. If this is the line with caret, save the result in m_LineExtentCache
3. Call ExtTextOut to draw the text. The function uses widths of each character, while GetTextExtentExPoint returns the accumulated extent.
