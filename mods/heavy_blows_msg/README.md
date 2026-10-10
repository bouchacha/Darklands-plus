# heavy_blows_msg
Example of a new string in a mod: same as heavy_blows plus a string stored in the code segment
(static char _based(_segname("_CODE")) ...), which needs no DGROUP space. Build-tested only.
