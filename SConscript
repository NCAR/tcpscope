# -*- python -*-

tools = Split("""
qt4
ascope
qtt_qtconfig
doxygen
boost_program_options
""")

env = Environment(tools = ['default'] + tools)

env.Append(LIBS = ['radar', 'rapmath', 'rapformats', 'Radx', 'Fmq', 
                   'dsserver', 'didss', 'toolsa', 'dataport', 'Spdb'])

sources = Split("""
main.cpp
AScopeReader.cpp
""")

headers = Split("""
AScopeReader.h
""")

html = env.Apidocs(sources + headers)

tcpscope = env.Program('tcpscope', sources)

#Default(tcpscope, html)
Default(tcpscope)
