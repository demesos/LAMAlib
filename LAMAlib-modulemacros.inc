.ifndef MODULE_MACROS_inc

MODULE_MACROS_inc=1

.macro m_init m_name1,m_name2,m_name3,m_name4,m_name5,m_name6,m_name7,m_name8,m_name9
	jsr m_name1::init
	.ifnblank m_name2
	  m_init m_name2,m_name3,m_name4,m_name5,m_name6,m_name7,m_name8,m_name9
	.endif
.endmacro

.macro m_run m_name1,m_name2,m_name3,m_name4,m_name5,m_name6,m_name7,m_name8,m_name9
	jsr m_name1::run
	.ifnblank m_name2
	  m_init m_name2,m_name3,m_name4,m_name5,m_name6,m_name7,m_name8,m_name9
	.endif

.endmacro

.macro m_call m_name,routine
	jsr m_name::routine
.endmacro

.macro def_const identifier,defaultvalue,warningmsg
.ifndef identifier
  .if .paramcount=1
    .error .sprintf("value for %s must be defined in main program",.string(identifier))
    .exitmacro
  .endif
  identifier=defaultvalue
.endif
.endmacro

.endif


