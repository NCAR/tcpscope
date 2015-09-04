# -*- python -*-

tools = Split("""
qt4
ascope
qtt_qtconfig
rapradar
doxygen
boost_program_options
rapradar
""")

env = Environment(tools = ['default'] + tools)

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
