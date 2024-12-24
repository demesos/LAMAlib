;reserves two bytes from zeropage for displayPETSCII routines

.zeropage
_displaypetscii_zptr: .res 2 

.exportzp _displaypetscii_zptr
